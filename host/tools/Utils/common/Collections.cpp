//####COPYRIGHTBEGIN####
//                                                                          
// ----------------------------------------------------------------------------
// Copyright (C) 1998, 1999, 2000 Red Hat, Inc.
//
// This program is part of the eCos host tools.
//
// This program is free software; you can redistribute it and/or modify it 
// under the terms of the GNU General Public License as published by the Free 
// Software Foundation; either version 2 of the License, or (at your option) 
// any later version.
// 
// This program is distributed in the hope that it will be useful, but WITHOUT 
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
// more details.
// 
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 
// 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
// ----------------------------------------------------------------------------
//                                                                          
//####COPYRIGHTEND####
#include "Collections.h"

void String::Format (LPCTSTR  const pszFormat,...)
{
  va_list args;
  va_start(args, pszFormat);
  vFormat(pszFormat,args);
  va_end(args);
}

void String::vFormat(LPCTSTR  pszFormat, va_list marker)
{
  for(int nLength=100;nLength;) {
    TCHAR *buf=new TCHAR[1+nLength];
    int n=_vsntprintf(buf, nLength, pszFormat, marker ); 
    if(-1==n){
      nLength*=2;  // NT behavior
    } else if (n<nLength){
      *this=buf;
      nLength=0;   // trigger exit from loop
    } else {
      nLength=n+1; // UNIX behavior generally, or NT behavior when buffer size exactly matches required length
    }
    delete [] buf;
  }
}

TCHAR * String::GetBuffer(unsigned int nLength)
{
  assert(NULL==m_pszBuf);
  nLength=MAX(nLength,GetLength()); // to accommodate _tcscpy below
  m_pszBuf=new TCHAR[1+nLength];
  m_nBufferLength=nLength;
  _tcscpy(m_pszBuf,c_str());
  return m_pszBuf;
}

void String::ReleaseBuffer()
{
  assert(m_pszBuf);
  m_pszBuf[m_nBufferLength]=_TCHAR('\0'); // just in case the terminating null has been forgotten
  *this=m_pszBuf;
  delete [] m_pszBuf;
  m_pszBuf=0;
}

String String::CStrToUnicodeStr(const char *psz)
{
  String str;
#ifdef _UNICODE
  int nLength=1+strlen(psz);
  MultiByteToWideChar(CP_ACP, 0, psz, -1, str.GetBuffer(nLength), nLength);
  str.ReleaseBuffer();
#else
  str=psz;
#endif
  return str;
}

int String::Chop(StringArray &ar,TCHAR cSep,bool bObserveStrings/*=TRUE*/) const
{
  assert('\0'!=cSep);
#define IsSep(c) (cSep==_TCHAR(' ')?isspace(c):c==cSep)
  LPCTSTR c=c_str();
  ar.clear();
  for(;;){
    // Skip multiple separators
    while(IsSep(*c))c++;
    if(*c==_TCHAR('\0')){
      break;
    }
    String strTok;
    if(bObserveStrings){
      bool bInString=false;
      do{
        if(*c==_TCHAR('\\') && c[1]){
          strTok+=c[1];
          c++;
        } else if(*c==_TCHAR('"')){
          bInString ^= 1;
        } else if (!bInString && IsSep(*c)) {
          break;
        } else {
          strTok+=*c;
        }
      } while (*++c);
    } else {
      do {
        if(IsSep(*c)) {
          break;
        } else {
          strTok+=*c;
        }
      } while (*++c);
    }
    ar.push_back(strTok);
  }
  return ar.size();
}

char * String::GetCString() const
  {
    char *psz=new char[1+GetLength()];
#ifdef _UNICODE
    WideCharToMultiByte(CP_ACP, 0, c_str(), -1, psz, 1+GetLength(), NULL, NULL);
#else
    strcpy(psz,c_str());
#endif
    return psz;
  }
  

