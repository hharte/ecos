//==========================================================================
//
//      ledit.c
//
//      Utterly simple line editor
//
//==========================================================================
//####COPYRIGHTBEGIN####
//                                                                          
// -------------------------------------------                              
// The contents of this file are subject to the Red Hat eCos Public License 
// Version 1.1 (the "License"); you may not use this file except in         
// compliance with the License.  You may obtain a copy of the License at    
// http://www.redhat.com/                                                   
//                                                                          
// Software distributed under the License is distributed on an "AS IS"      
// basis, WITHOUT WARRANTY OF ANY KIND, either express or implied.  See the 
// License for the specific language governing rights and limitations under 
// the License.                                                             
//                                                                          
// The Original Code is eCos - Embedded Configurable Operating System,      
// released September 30, 1998.                                             
//                                                                          
// The Initial Developer of the Original Code is Red Hat.                   
// Portions created by Red Hat are                                          
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.                             
// All Rights Reserved.                                                     
// -------------------------------------------                              
//                                                                          
//####COPYRIGHTEND####
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):    
// Contributors: gthomas
// Date:         1999-10-20
// Purpose:      Udderly simple line editor
// Description:  
//               
//
//####DESCRIPTIONEND####
//
//=========================================================================


#include <monitor.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_BSP
#include <bsp/bsp.h>
#endif
#include "ledit.h"

#ifndef NULL
#define NULL 0
#endif

static char *cutBuf = NULL;

#ifdef NO_MALLOC
static char linebufArray [MAX_HIST_ENTS + 2][MAXLINELEN + 1];
#endif

static struct termcap 
{
  char *relleft, *relright;
  char *oneleft;
  char *insertch;
  char *deletech;
  char *deleteonech;
  char *clreol;
  char *gobol;
  int width;
} terminal;

static struct linepos 
{
  char *prompt;
  char *buffer;
  char *ebuf;
  int cursor;
} linebuf;

static struct history 
{
  char *cmd;
  struct history *next, *prev;
} *list = NULL, *topl = NULL, *curl = NULL;

static int histlen = 0;
static int histLimit = MAX_HIST_ENTS;

static struct history histEnts[MAX_HIST_ENTS + 1], *histPtr = histEnts;
static struct history currLine = { NULL, NULL, NULL };

void
beep (void)
{
  xputchar ('\007');
}

void
printHistoryList () 
{
  struct history *hist = list;
  int hist_num = 1;

  if (hist != NULL)
  {
    while (hist->prev != NULL)
    {
      hist = hist->prev;
    }

    while (hist != NULL)
    {
      if (hist->cmd != NULL)
        xprintf(" %d %s\n", hist_num++, hist->cmd);
      hist = hist->next;
    }
  }
}

static void
outputParamStr (char *str, int val)
{
  char *i = index (str, '%');
  char *ptr;
  int dist = val;

  if (i == NULL)
    {
      while (dist-- > 0)
	xprintf (str);
    }
  else
    {
      for (ptr = str; *ptr && ptr < i; ptr++)
	xputchar (*ptr);
      if (dist > 99)
	{
	  xputchar ('0' + dist / 100);
	  dist = dist % 100;
	}
      if (dist > 9)
	{
	  xputchar ('0' + dist / 10);
	  dist = dist % 10;
	}
      xputchar ('0' + dist);
      if (*ptr)
	xprintf (ptr + 1);
    }
}

static void
absMoveCursor (int pos)
{
  int dist, oldpos = linebuf.cursor;
  int absdist;
  char *bigmove;

  if (pos > (linebuf.ebuf - linebuf.buffer))
    {
      beep ();
      pos = linebuf.ebuf - linebuf.buffer;
    }
  else if (pos < 0)
    pos = 0;
  dist = pos - linebuf.cursor;
  absdist = (dist < 0 ? -dist : dist);
  linebuf.cursor = pos;
  if (dist == 0)
    return;
  if (dist < 0)
    bigmove = terminal.relleft;
  else
    bigmove = terminal.relright;

  if ((absdist < 4) || (bigmove == NULL))
    {
      int x;
      int promptLen = strlen (linebuf.prompt);

      if (pos < (absdist - promptLen))
	{
	  xprintf (terminal.gobol);
	  xprintf (linebuf.prompt);
	  for (x = 0; x < pos; x++)
	    xputchar (linebuf.buffer[x]);
	  return;
	}

      if (dist < 0)
	{
	  for (x = 0; x < -dist ;x++)
	    xprintf (terminal.oneleft);
	}
      else
	{
	  for (x = 0; x < dist; x++)
	    xputchar (linebuf.buffer [oldpos + x]);
	}
    }
  else
    {
      outputParamStr (bigmove, absdist);
    }
}

static void
clrScrToEol ()
{
  int len = linebuf.ebuf - linebuf.buffer;

  if (len < linebuf.cursor)
    return;

  if(terminal.clreol)
    {
      xprintf (terminal.clreol);
    }
  else if (terminal.deletech)
    {
      outputParamStr (terminal.deletech, len - linebuf.cursor);
    }
  else
    {
      int oldcur = linebuf.cursor;
      while (linebuf.cursor < len)
	{
	  xputchar (' ');
	  linebuf.cursor++;
	}
      
      absMoveCursor (oldcur);
    }
}

static void
redrawCmd ()
{
  xprintf (terminal.gobol);
  xprintf (linebuf.prompt);
  linebuf.buffer[linebuf.cursor] = 0;
  xprintf (linebuf.buffer);
  clrScrToEol ();
}

static void
instCmd (char *ncmd)
{
  linebuf.cursor = strlen (ncmd);
  strcpy (linebuf.buffer, ncmd);
  redrawCmd ();
  linebuf.ebuf = linebuf.buffer + linebuf.cursor;
}

static void
prevCmd () 
{
  if (curl == &currLine)
    {
      if (list != NULL)
	{
	  *linebuf.ebuf = 0;
#ifdef NO_MALLOC
	  currLine.cmd = linebufArray[MAX_HIST_ENTS];
	  strcpy (currLine.cmd, linebuf.buffer);
#else
	  if (currLine.cmd != NULL)
	    free (currLine.cmd);
	  currLine.cmd = strdup (linebuf.buffer);
#endif
	  curl = list;
	}
    }
  else
    {
      if (curl->prev != NULL)
	curl = curl->prev;
    }
  if (curl != NULL && curl->cmd != NULL)
    instCmd (curl->cmd);
  else
    beep ();
}

static void
nextCmd ()
{
  if (curl->next == NULL)
    {
      beep ();
    }
  else
    {
      curl = curl->next;
      instCmd (curl->cmd);
    }

}

static int initted = 0;

void
initVt100 () 
{
  terminal.gobol = "\r";
  terminal.oneleft = "\010";
  terminal.relleft = "\033[%D";
  terminal.relright = "\033[%C";
  terminal.insertch = "\033[%@";
  terminal.deletech = "\033[%P";
  terminal.deleteonech = "\033[P";
  terminal.clreol = "\033[K";
  terminal.width = 80;
  initted = 1;
}

void
initDumb ()
{
  terminal.gobol = "\r";
  terminal.oneleft = "\010";
  terminal.relleft = NULL;
  terminal.relright = NULL;
  terminal.insertch = NULL;
  terminal.deletech = NULL;
  terminal.deleteonech = NULL;
  terminal.clreol = NULL;
  terminal.width = 80;
  initted = 1;
}

static void
insertChar (char *chars, int num)
{
  int len = linebuf.ebuf - linebuf.buffer + strlen (linebuf.prompt);
  int n = 0;

  if ((len + num) >= terminal.width)
    {
      beep ();
      return;
    }
  if ((linebuf.ebuf - linebuf.buffer) > linebuf.cursor)
    {
      char *ptr, *eptr = linebuf.buffer + linebuf.cursor;

      for (ptr = linebuf.ebuf; ptr >= eptr; ptr--)
	*(ptr+num) = *ptr;

      if (terminal.insertch != NULL)
	outputParamStr (terminal.insertch, num);
    }
  for (n = 0; n < num; n++)
    {
      xputchar (*chars);
      linebuf.buffer[linebuf.cursor++] = *(chars++);
    }

  linebuf.ebuf += num;

  if (terminal.insertch == NULL)
    {
      char *ptr = linebuf.buffer + linebuf.cursor;
      int oldcur = linebuf.cursor;
      for (; ptr < linebuf.ebuf; ptr++)
	xputchar (*ptr);
      linebuf.cursor = linebuf.ebuf - linebuf.buffer;
      absMoveCursor (oldcur);
    }
}

static void
deleteEol (int putInCutBuffer)
{
  int len = linebuf.ebuf - linebuf.buffer;
  if (linebuf.cursor < len)
    {
      clrScrToEol ();

      if (putInCutBuffer)
	{
	  *linebuf.ebuf = 0;
#ifdef NO_MALLOC
	  cutBuf = linebufArray[MAX_HIST_ENTS + 1];
	  strcpy (cutBuf, linebuf.buffer + linebuf.cursor);
#else
	  if (cutBuf != NULL)
	    free (cutBuf);
	  cutBuf = strdup (linebuf.buffer + linebuf.cursor);
#endif
	}
      linebuf.ebuf = linebuf.buffer + linebuf.cursor;
    }
}

static void
deleteCurrChar ()
{
  int len = linebuf.ebuf - linebuf.buffer;
  char *ptr;
  if (len == linebuf.cursor || len == 0)
    return;
  for (ptr = linebuf.buffer + linebuf.cursor; ptr < (linebuf.ebuf - 1); ptr++)
    {
      *ptr = *(ptr + 1);
      if (terminal.deleteonech == NULL)
	xputchar (*ptr);
    }
  linebuf.ebuf--;
  if (terminal.deleteonech && (len - 1) != linebuf.cursor)
    xprintf (terminal.deleteonech);
  else
    {
      int oldcur = linebuf.cursor;
      xputchar (' ');
      linebuf.cursor = linebuf.ebuf - linebuf.buffer + 1;
      absMoveCursor (oldcur);
    }
}

static void
deleteChar ()
{
  if (linebuf.cursor == 0)
    {
      beep ();
      return;
    }
  absMoveCursor (linebuf.cursor - 1);
  deleteCurrChar ();
}

int
lineedit (char *prompt, char *buffer, int maxLen)
{
  int c;

  curl = &currLine;

  if (!initted)
    {
      initted = 1;
      /*initVt100 ();*/
      initDumb();
    }
  linebuf.prompt = prompt;
  linebuf.buffer = buffer;
  linebuf.ebuf = buffer;
  buffer[0] = 0;
  linebuf.cursor = 0;
  redrawCmd ();
  while ((c=input_char ()) > 0) 
    {
      switch (c)
	{
	case PREVCMD:
	  prevCmd ();
	  break;
	case NEXTCMD:
	  nextCmd ();
	  break;
	case LF:
	case CR:
	  *linebuf.ebuf = 0;
#ifdef NO_MALLOC
	  cutBuf = NULL;
	  currLine.cmd = NULL;
#else
	  if (cutBuf != NULL)
	    {
	      free (cutBuf);
	      cutBuf = NULL;
	    }
	  if (currLine.cmd != NULL)
	    {
	      free (currLine.cmd);
	      currLine.cmd = NULL;
	    }
#endif
	  return linebuf.ebuf - linebuf.buffer;
	  break;
	case BOLCMD:
	  absMoveCursor (0);
	  break;
	case EOLCMD:
	  absMoveCursor (linebuf.ebuf-linebuf.buffer);
	  break;
	case FORWCMD:
	  absMoveCursor (linebuf.cursor + 1);
	  break;
	case BACKCMD:
	  absMoveCursor (linebuf.cursor - 1);
	  break;
	case DELBACK:
	case '\177':
	  deleteChar ();
	  break;
	case ERASELINE:
	  absMoveCursor (0);
	  deleteEol (0);
	  break;
	case DELEOL:
	  deleteEol (1);
	  break;
	case DELCURRCH:
	  deleteCurrChar ();
	  break;
	case YANKCH:
	  if (cutBuf != NULL)
	    insertChar (cutBuf,strlen (cutBuf));
	  break;
	default:
	  if (c >= 32 && c < 127)
	    {
	      char ch = c;
	      insertChar (&ch, 1);
	    }
	  break;
	}
    }
  return -1;
}

void
addHistoryCmd (char *cmd)
{
  struct history *newent = NULL;

  if (histlen >= histLimit)
    {
      newent = topl;
      topl = topl->next;
      topl->prev = NULL;
#ifdef NO_MALLOC
      newent->prev = NULL;
      newent->next = NULL;
#else
      free (newent->cmd);
      newent->cmd = NULL;
#endif
      histlen = histLimit - 1;
    }

  histlen++;

  if (newent == NULL)
    {
      newent = histPtr++;
#ifdef NO_MALLOC
      newent->cmd = linebufArray[histlen - 1];
#endif
    }

  if (list == NULL)
    {
      list = newent;
      list->prev = NULL;
      topl = list;
    }
  else
    {
      list->next = newent;
      list->next->prev = list;
      list = list->next;
    }
  currLine.prev = list;
  list->next = &currLine;
#ifdef NO_MALLOC
  strcpy (list->cmd, cmd);
#else
  list->cmd = strdup (cmd);
#endif
  curl = &currLine;
}

void
set_term_name (char *name)
{
  if (! strcmp (name, "vt100"))
    {
      initVt100 ();
    }
  else if (! strcmp (name, "dumb"))
    {
      initDumb ();
    }
  else
    {
      xprintf ("Unknown terminal name %s\n", name);
    }  
}
