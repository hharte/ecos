//==========================================================================
//
//        signal1.cxx
//
//        POSIX signal test 1
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
// Author(s):     nickg
// Contributors:  nickg
// Date:          2000-04-10
// Description:   Tests POSIX signal functionality.
//
//####DESCRIPTIONEND####
//==========================================================================

#include <sys/types.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>

#include <cyg/infra/testcase.h>

//--------------------------------------------------------------------------
// Thread stack.

char thread_stack[PTHREAD_STACK_MIN*2];

//--------------------------------------------------------------------------
// Local variables

// Sync semaphore
sem_t sem;

// Thread ID
pthread_t thread1;

volatile int sigusr2_called = 0;
volatile int sigalrm_called = 0;

//--------------------------------------------------------------------------
// Signal handler functions

static void sigusr2( int signo )
{
    CYG_TEST_INFO( "sigusr2() handler called" );
    CYG_TEST_CHECK( signo == SIGUSR2, "Signal not SIGUSR2");
    CYG_TEST_CHECK( pthread_equal(pthread_self(), thread1), "Not called in thread1");

    sigusr2_called++;
}

static void sigalrm( int signo )
{
    CYG_TEST_INFO( "sigalrm() handler called" );
    CYG_TEST_CHECK( signo == SIGALRM, "Signal not SIGALRM");
    CYG_TEST_CHECK( pthread_equal(pthread_self(), thread1), "Not called in thread1");

    sigalrm_called++;
}

//--------------------------------------------------------------------------

void *pthread_entry1( void *arg)
{
    sigset_t mask;
    siginfo_t info;
    struct timespec timeout;
    int sig;
    
    CYG_TEST_INFO( "Thread 1 running" );

    // Make a full set
    sigfillset( &mask );

    // remove USR2 and ALRM signals
    sigdelset( &mask, SIGUSR2 );
    sigdelset( &mask, SIGALRM );

    // Set signal mask
    pthread_sigmask( SIG_SETMASK, &mask, NULL );
    
    // Get main thread going again
    sem_post( &sem );

    // set up timeout
    timeout.tv_sec = 10;
    timeout.tv_nsec = 0;

    CYG_TEST_INFO( "Thread1: calling sigtimedwait()");
    
    // Wait for a signal to be delivered
    sigtimedwait( &mask, &info, &timeout );

    sig = info.si_signo;
    
    CYG_TEST_CHECK( sig == SIGUSR1, "Signal not delivered");

    while( sigusr2_called != 2 )
    {
        CYG_TEST_INFO( "Thread1: calling pause()");        
        pause();
    }

    // now wait for SIGALRM to be delivered
    CYG_TEST_INFO( "Thread1: calling pause()");            
    pause();

    CYG_TEST_INFO( "Thread1: calling pthread_exit()");    
    pthread_exit( (void *)((int)arg+sig) );
}

//--------------------------------------------------------------------------

int main(int argc, char **argv)
{
    int ret;
    sigset_t mask;
    pthread_attr_t attr;
    void *retval;
    union sigval value;
    
    CYG_TEST_INIT();

    // Make a full signal set
    sigfillset( &mask );

    
    // Install signal handlers
    {
        struct sigaction sa;

        sa.sa_handler = sigusr2;
        sa.sa_mask = mask;
        sa.sa_flags = 0;

        ret = sigaction( SIGUSR2, &sa, NULL );

        CYG_TEST_CHECK( ret == 0 , "sigaction returned error");
    }

    {
        struct sigaction sa;

        sa.sa_handler = sigalrm;
        sa.sa_mask = mask;
        sa.sa_flags = 0;

        ret = sigaction( SIGALRM, &sa, NULL );

        CYG_TEST_CHECK( ret == 0 , "sigaction returned error");
    }
    
    
    // Mask all signals
    pthread_sigmask( SIG_SETMASK, &mask, NULL );
    
    sem_init( &sem, 0, 0 );
    
    // Create test thread
    pthread_attr_init( &attr );

    pthread_attr_setstackaddr( &attr, (void *)&thread_stack[sizeof(thread_stack)] );
    pthread_attr_setstacksize( &attr, sizeof(thread_stack) );

    pthread_create( &thread1,
                    &attr,
                    pthread_entry1,
                    (void *)0x12345678);

    // Wait for other thread to get started
    CYG_TEST_INFO( "Main: calling sem_wait()");
    sem_wait( &sem );

    value.sival_int = 0;

    // send a signal to the other thread
    CYG_TEST_INFO( "Main: calling sigqueue(SIGUSR1)");
    sigqueue( 0, SIGUSR1, value );

    // Send the signal via kill
    CYG_TEST_INFO( "Main: calling kill(0, SIGUSR2)");
    kill( 0, SIGUSR2 );

    // Wait for thread1 to call pause()
    CYG_TEST_INFO( "Main: calling sleep(1)");
    sleep(1);

    // And again
    CYG_TEST_INFO( "Main: calling kill(0, SIGUSR2)");
    kill( 0, SIGUSR2 );

    // Set up an alarm for 1 second hence
    CYG_TEST_INFO( "Main: calling alarm(1)");
    alarm(1);
    
    // Wait for alarm signal to be delivered to thread1
    CYG_TEST_INFO( "Main: calling sleep(2)");
    sleep(2);
    
    // Now join with thread1
    CYG_TEST_INFO( "Main: calling pthread_join()");
    pthread_join( thread1, &retval );

    CYG_TEST_CHECK( sigusr2_called == 2, "SIGUSR2 signal handler not called twice" );

    CYG_TEST_CHECK( sigalrm_called == 1, "SIGALRM signal handler not called" );    
    
    // check retval
    
    if( (long)retval == 0x12345678+SIGUSR1 )
        CYG_TEST_PASS_FINISH( "signal1" );
    else
        CYG_TEST_FAIL_FINISH( "signal1" );
}

//--------------------------------------------------------------------------
// end of signal1.c
