############### All NonInteractive processes, same worktime ########
                3 processes-> 1 of which has a spawn block of 5 child processes
1.conf:
    Round Robin: 4030ms
    SJF Goodness: 4000ms
    SJF Exp Burst: 3499ms
    SJF Standard Goodness: 2749ms
    Average Finish Time:
        Round Robin: 3898ms
        SJF Goodness: 3929ms
        SJF Exp Burst: 2852ms
        SJF Standard Goodness: 2023ms
    
    The results are expected. This is the worst case for a Round Robin algorithm, because all processes have the same worktime.
    This leads to them finishing almost prett close to each other, thus increasing the total average time.
    SJF with Goodness algorithm seems to work very similar RR->giving each process same time in turns. That could be because 
    it takes in mind the time a process waits in readyqueue, so since all processes spawn pretty close to one another, that time could be
    similar for each process, so it works in turns. The only difference with RR is that when the spawned child processes C and D from 
    base process spawner are created, SJF with Goodness gives priority to them so they finish faster. Instead RR spends time to switch 
    back to A and B process. But after that point, SJF with Goodness and RR are the same, since the waiting time in the readyqueue comes to be 
    the same for all processes.
    SJF Exp_Burst and SJF with standard Goodness implementations both work similar. They seem to give a pseudo-priority to the spawned processes,
    spending less time switching. This is because these implementations dont take in mind the waiting time of the other processes and just "focus"
    on finishing the ones that seem to have a bigger priority. This beats the only weakness of SJF  Goodness that taks that time in mind and finishes 
    slow. However, we see SJF Standard Goodness is significantly faster. That could be because it has less switches and gives more time for the processes 
    to run. Exp_Burst works similar, but at some point starts schedule a lot of processes and keeps switching between them.
    
    We can see with this profile the weakness in RR and the worst case in SJF Goodness, as well as the significantly better results with SJF Standard Goodness
    and SJF Exp_Burst.
    
    
    
################## All Interactive, same spawn time ######################################
                    4 processes -> no spawn

2.conf->All interactive, processes with same time:
    Round Robin: 1279ms
    SJF Goodness: 1279ms
    SJF Exp Burst: 1979ms
    SJF Standard Goodness: 1169ms 
    Average Finish Time:
        Round Robin: 864ms
        SJF Goodness: 849ms
        SJF Exp Burst: 1114ms
        SJF Standard Goodness: 779ms
    
    We can see that SJF Goodness switches the processes like RR (in turns). Also, the time a process goes to sleep or wakes up
    does not effect the schedulling-everything keeps working the same, which makes the performance smoother.
    SJF with Exp_Burst however, has a lower performance. A root cause for that is the sleeping processes. As soon as a process
    wakes up and enters the queue, its burst is considerably lower than the other processes, since they have been running for all
    this time this process was asleep. This leads to a different scheduling, wasting time.
    
    Concluding we can say that SJF performs well on average time when all processess spawn at the same time.
    For Interactive processes, SJF Goodness has an advantage to the other implementations, since the return of 
    a sleeping process does not affect the scheduling, thus wastin less time.
    
##################### Complex Profile, nested spawn processes, one with bigger worktime ##########################
                1 NonInteractive process with 2 immediate children.
                1 child has 1 child and 1 grandchild.
                The other child has 4 times the worktime.
                5 processes in total.
                
3.conf:
    Round Robin: 780ms
    SJF Goodness: 1184ms
    SJF Exp Burst: 947ms
    SJF Standard Goodness: 1025ms 
        Average Finish TIme:
        Round Robin: 864ms
        SJF Goodness: 849ms
        SJF Exp Burst: 1114ms
        SJF Standard Goodness: 779ms
    
    
    
    
    
############## Complex Profile, one very long process ########################
    
4.conf->Longest processes spawn first:
    Round Robin: 8209ms
    SJF Goodness: 8112ms
    SJF Exp Burst: 8596ms
    SJF Standard Goodness: 8126ms 
    
    The disadvantage of SJF is that long processes may never be processed by the system and may
    remain in the queue for very long time leading to starvation of processes.
    Here we have the worst case for SJF Exp_Burst where there are a lot of processes with low worktime and
    on with much more. The disadvantage of this algorithm is that this long process may never be processed 
    by the system and may remain in the queue for very long time leading to starvation of processes.
    
############## One Non Interactive A and one Interactive B ####################
                2 proceesses->no spawn
5.conf:
        Round Robin: 1479ms
        SJF Goodness: 1579ms*
        SJF Exp Burst: 1579ms*
        SJF Standard Goodness: 1579ms *
    Average Finish TIme:
        Round Robin: 1099ms
        SJF Goodness: 1141ms*
        SJF Exp Burst: 1141ms*
        SJF Standard Goodness: 1141ms*
    *There was a lot of time spent idle, as process A(noninteractive) had finished
        while process B(interactive, only one left) was sleeping. Generally, process A with SJF always
        finishes earlier than processs A with Round Robin, but there just happens to be a very big
        offset when B is asleep, which adds time to the overall time with algorithm SJF.
        If we take away that time:
            Round Robin:(852-728) + (1430-1098) = 456 of idle time.
            where 852ms is the time B wakes up and 728 the time it goes to sleep.
            Then likewise 1430 is the next time it wakes up and 1098 when it went to sleep.
            SJF: (994-716) + (1572-1248) = 602 of idle time.
        
        So the actual times should be:
            Round Robin: 1023ms
            SJF Goodness: 977ms
            
        which shows that SJF is faster in clean time than RR. Also, another overhead for RR in this case,
        is that theres extra time added for every switch when both A and B are awake. This is the main disadvantage 
        of RR in this case. While SJF doesnt have this issue, since hen both processes are awake, it gives priority to
        the interactive one and as it goes to sleep the noninteractive one takes the CPU. This is why it runs faster
        (in clean time).
