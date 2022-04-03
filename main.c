// Author: T00647822 - James Nguyen  

#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/times.h>
#include <unistd.h>
#include <sys/resource.h>
#include<ncurses.h>            
#define NANO2SEC 1000000000 


//declare struct & global variables
struct rusage r_usage;
struct timespec gettimenow;
double getWtime;
double getCtick;
int ncore;
double cpu_util;

//--------------------------------------------------------
//                FUNCTION 
//-------------------------------------------------------- 

// Get Wall-time ( actual time start of the process and 'now' ) 
double get_wall_time () {
  if (clock_gettime(CLOCK_REALTIME,&gettimenow)){
      //handle error 
      return 0;
  }
  return ( (double)gettimenow.tv_sec + ( (double)gettimenow.tv_nsec / NANO2SEC ) );
}

// Get CPU Time | Total Processsing Time, ignores I/O time and other processes (seconds) 
double get_cpu_time () {
  return ( (double)clock() / sysconf (_SC_CLK_TCK));
}

// Get number of core in the sys. 
int core_logical () {
  return (sysconf(_SC_NPROCESSORS_ONLN));
}


// Get Avg CPU Utilization (multicore). 
double get_cpu_ult () {
    long double a[4], b[4], loadavg;
    FILE *fp;
    char dump[50];
    //get first idletime & uptime 
    fp = fopen("/proc/stat","r");
    fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&a[0],&a[1],&a[2],&a[3]);
    fclose(fp);
    
    sleep(1); //pause 1s 
    
    //get 2nd idletime & uptime 
    fp = fopen("/proc/stat","r");
    fscanf(fp,"%*s %Lf %Lf %Lf %Lf",&b[0],&b[1],&b[2],&b[3]);
    fclose(fp);

    // => to get CPU Ult: 100 - 100 * (idle2 - idle1) / (uptime2 - uptime1)
    loadavg = 100 - 100 *((b[0]+b[1]+b[2]) - (a[0]+a[1]+a[2])) / ((b[0]+b[1]+b[2]+b[3]) - (a[0]+a[1]+a[2]+a[3]));

    return loadavg;
}


//--------------------------------------------------------
//                MAIN  
//-------------------------------------------------------- 
int main (int argc, char *argv[]) {
    bool tracking = true;
    
    // starts ncurses 
    initscr(); 
    /* init window */
    WINDOW *win = newwin(15, 70, 2, 10); // 15hx70w
    keypad(stdscr, TRUE); // to use arrow keys
    clear();
	noecho();
	cbreak();	// Line buffering disabled. pass on everything 
	curs_set(0);// turn off cursor 
	
    // making box border with default border styles
    box(win, 0, 0);
    
    /*color window */
    start_color();
    init_pair(1, COLOR_MAGENTA, COLOR_WHITE);
    wbkgd(win, COLOR_PAIR(1));
    
    mvwprintw(win, 0, 5, "[ Resource Monitor ]");
    mvwprintw(win, 14, 55,"[Exit: x]");
    wtimeout(win,1000);
        
    bool isActive = true; 
    while (isActive == true) { 
        // refreshing the window
        refresh();
        /* retrive data and print (update) in window. */
        cpu_util = get_cpu_ult(); 
        mvwprintw(win, 2, 2,"CPU");
        mvwprintw(win, 3, 2," > CPU Utilization: %.2f %% ", cpu_util);
        getWtime = get_wall_time ();
        int sec, hh, mm, ss;
        sec = getWtime;
        hh = sec/3600;
	    mm = (sec - hh*3600)/60;
	    ss = sec - hh*3600 - mm*60;
        mvwprintw(win, 4, 2," > Up time: %d hours : %d minutes : %d seconds", hh, mm, ss);
        getCtick = get_cpu_time ();
        mvwprintw(win, 5, 2," > CPU time: %.2f seconds", getCtick);
        ncore = core_logical ();
        mvwprintw(win, 6, 2," > No of cores: %d ", ncore);
        mvwprintw(win, 8, 2,"Memory");
        getrusage(RUSAGE_SELF,&r_usage);
        mvwprintw(win, 9, 2," > Memory usage: %ld kbs ", r_usage.ru_ixrss);
        getrusage(RUSAGE_SELF,&r_usage);
        mvwprintw(win, 10, 2," > Max Memory: %ld kbs ", r_usage.ru_maxrss);
       
        /* Get user choice*/
        char ch = wgetch(win); 
        if ( ch == 'x'){ //if input is X then Exit  
            isActive = false; 
            break;
        } 
    }
    getch();
    endwin();
  return 0;
} 
