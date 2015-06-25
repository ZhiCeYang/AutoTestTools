Auto test tool:
Description:
  We designed this tool for our test plan, people can use this tool to 
control VOIP phones by sending key events, and it may help us to find 
those random bugs and help us to complete the Stress Test or Stability 
Test. The core of this tool is the application running at PC, we can use 
this APP to add, del or show clients which had connected to PC through
the network.
Compile and run:
  I had already run this tool with a platform for cygwin of 64bit, and
had not test on any other platforms. So how to get this executable binary
file(APP)? copy these source codes, and enter this folder through cygwin's
terminal. Enter a "make" command,then wait for compile complete.
  But here still some case we need to concern. Firstly,your cygwin must
support "make". secondly,if "bad system call" appeared when running the APP,
you should install the IPC service and start it,run "apt-cyg install cygserver"
to install if your cygwin support "apt-cyg","cygserver-config" can help you 
to complete the configuration, and "cygrunsrv -S cygserver" will help you 
to start this service. you can input command "ipcs" to test whether IPC 
server started or not, and it will prompt "bad system call" if the service
had not started successfully.
Notice:
  In this version, script support command "add","del","show" "ioevt","delay" ,
"randomTime", "randomKey", "" and "loop". if you need to use loop,you should 
tell it where to start and where is the end. you can't write a loop in a loop
or an ioevt with a key number greater than 10. Command "help" will tell you how 
to use those commands.

Operation guide:
1. enter folder "remoteAPI" by cygwin
2. execute "./autoTest.exe"
3. input command "start all" to start test mode and receive mode,if error 
   appeared, check your IPC server started or not. 
   we can also use command "autoConfig"(ac for short) to load a script to 
   complete our configuration. eg. ac config.txt
4. use command "add 1(client num:0-9) 192.168.11.16(ip addr)" to add a connection
   between phone and PC, 10 clients for MAX.
5. here you can control phones by sending IO event. For example: ioevt 1 1 p. 
   this command will tell phone 1 to input number 1 and enter dialling statue.
   'p' for key action, it means press key, and it will be convert to uiKeyDown
   and uiKeyUp in the phone's key events. similarly,'l' for long press.
6. you can also run scripts by command "runScript 1(script num:0-9) script.txt(script 
   file name, it must exist in folder remoteAPI)". We support multi scripts, 10 for
   MAX, and we should contain delay time in each scripts, otherwise, system can't 
   schedule between sub threads, then it seems like running a single script.
   '#' for comments, we can also add comments after the end flag ';'.
All supported KEY event definitions:
	"s1"    /*SOFT_KEY1*/
    "s2"    /*SOFT_KEY2*/
    "s3"    /*SOFT_KEY3*/
    "s4"    /*SOFT_KEY4*/
    "up"    /*KEY_ARROW_UP*/
    "dn"    /*KEY_ARROW_DOWN*/
    "1"     /*1*/
    "2"     /*2*/
    "3"     /*3*/
    "4"     /*4*/
    "5"     /*5*/
    "6"     /*6*/
    "7"     /*7*/
    "8"     /*8*/
    "9"     /*9*/
    "0"     /*0*/
    "*"     /* '*' */
    "#"     /* '#' */
    "vd"    /*KEY_ALL_VOL_DOWN*/
    "me"    /*KEY_MUTE*/
    "vu"    /*KEY_ALL_VOL_UP*/
    "rl"    /*KEY_REDIAL*/
    "he"    /*KEY_HANDFREE*/
    "le1"   /*LINE1*/
    "le2"   /*LINE2*/ 
All command list:
    {
        cmdName = "help"
        nickName = "h"
        example = "help: help/help"
    }
    {
        cmdName = "add"
        nickName = "ad"
        example = "add: add 1(client number) 192168101(ip address)"
    }
    {
        cmdName = "del"
        nickName = "dl"
        example = "del: del 1(client number)"
    }
    {
        cmdName = "changeIp"
        nickName = "ci"
        example = "changeIp: changeIp 1(client number) 192168101(ip address)"
    }
    {
        cmdName = "show"
        nickName = "s"
        example = "show: show"
    }
    {
        cmdName = "ioevt"
        nickName = "ie"
        example = "ioevt: ioevt 1(client number) 12(key number) p/l(action)"
    }
    {
        cmdName = "call"
        nickName = "call"
        example = "call: call 1(client number) 1000(tel number)"
    }
    {
        cmdName = "syslogLevel"
        nickName = "ssl"
        example = "setSysLogLevel: "
    }
    {
        cmdName = "start"
        nickName = "st"
        example = "start: ____test: start test\n\
         |_rec:  start rec\n\
         |_all:  start all"
    }
    {
        cmdName = "stop"
        nickName = "sp"
        example = "stop:  ____test: stop test\n\
         |_rec:  stop rec\n\
         |_rec:  stop 1(thread num) script\n\
         |_all:  stop all"
    }
    {
        cmdName = "quit"
        nickName = "q"
        example = "quit: quit"
    }
    {
        cmdName = "runScript"
        nickName = "rs"
        example = "runScript: runScript 1(thread Num) scripttxt(script file name)"
    }
    {
        cmdName = "delay"
        nickName = "dy"
        example = "delay: delay 5(secends)"
    }
    {
        cmdName = "randomTime"
        nickName = "rt"
        example = "randomTime: randomTime 1(min) 10(max)"
    }
    {
        cmdName = "randomKey"
        nickName = "rk"
        example = "randomKey: randomKey 1(client Num)"
    }
    {
        cmdName = "look"
        nickName = "l"
        example = "look: look"
    }
    {
        cmdName = "syslog"
        nickName = "sl"
        example = "syslog: syslog 1(client) enable/disable"
    }
    {
        cmdName = "autoConfig"
        nickName = "ac"
        example = "autoConfig: autoConfig configtxt"
    }
Script examples:
1.IO event:
  	loop 100;
	start;
	ioevt 1 he,1,0,2,s4 p;//add comments here
	# add comments here
	delay 1;
	end;
this script will tell client 1 to call 102 for 100 times.
2.randomKey:
	loop 100;
	start;
	randomKey 1; //add comments here
	# add comments here
	delay 1;
	end;
this script will loop 100 times and send a randomKey to client 1 each time.
3.autoConfig:
	ad 1 192.168.10.10;
	ad 2 192.168.10.11;
	st all;
	rs 1 script_randomKey.txt;
this script will add client 1 and 2 for phones which with the IP address as 
192.168.10.10 and 192.168.10.11. Start all(test and receive mode)  then let
client 1 to do the random key test.

Modify on 2014-12-9:
1.	now auto test tools can record our operation on the phone, all you need to do
	is just open it, if you enter the commands "syslog 1 enable" and "syslog 1 record",
	tools will create a file named (ip)_record (such as: 192.168.10.101_record) in
	the log folder, so we can use it as a script.
2.	test will stop if any task was suspended on the phone.
3.	we can also specify the log module we want, such as if you just want 
	the logs of screen module, you can enter a command "syslog 1 screen only".
