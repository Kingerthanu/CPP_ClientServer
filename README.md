# CPP_ClientServer
User Can Launch The Program In Multiple Modes; One Of These Being A Server Mode In Which Will Establish A Accepting Socket For Clients To Request From. The Other Modes Are Client-Side And Work On Reads Of A Given File Path, Checks On The File Path, Deletion Of A Given File Path, As Well As STDIO Reads And Then Writes To The Server Through A Byte Stream.


----------------------------------------------------------------------------

<img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/ef3cd650-187b-4eee-a10e-519f8f4a7355" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/ef3cd650-187b-4eee-a10e-519f8f4a7355" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/ef3cd650-187b-4eee-a10e-519f8f4a7355" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/ef3cd650-187b-4eee-a10e-519f8f4a7355" alt="Cornstarch <3" width="55" height="49"> 


**The Breakdown:**

Program Works With A Terminal In Order To Do Transactions Amongst A Host And Clients On File Directories Concurrently Through Multithreading.

The Program Starts By Calling It With An Attached Mode In Which Will Specifify The Specific Pipeline This Process Will Run Through.

There Will Be 1 Main Mode That Must Be Active In Order For The Others To Properly Work In Which Is The Server; The Server Will Arbitraily Ping A Port In Which To Bind The Processes' To, Creating A Listening Socket And Printing It. This Socket Will Work To Listen For A Incoming Request From A Client In Which Wants To Do A Transaction. The Server When Hearing A Connection Through Its Listen(...) Will Then Wait Until The Client Provides A Byte Stream Of The Given Mode And Its Size. With This Mode The Server Process Will Interpolate What Type Of Request The Client Is Trying To Do. This Could Be:

  - read \<IP\> \<Socket\> \<File Path\>: This Mode Will Allow A Client To Request The Server To Look Through Its Relative Directory For The Given File Path. With This File--If Created--Will Provide The Client With A Byte-Stream Of The Files Contents And Print Out The Contents.

  - check \<IP\> \<Socket\> \<File Path\>: This Mode Will Allow A Client To Request The Server To Look Through Its Relative Directory For The Given File Path And If It's Currently Created Will Tell The User Through A Simple Byte-Boolean.

 - delete \<IP\> \<Socket\> \<File Path\>: This Mode Will Allow A Client To Request The Server To Look Through Its Relative Directory For The Given File Path And If It's Currently Created Will Delete The File From The Server's Relative Directory And Tell The User Through A Simple Byte-Boolean.

 - store \<IP\> \<Socket\> \<File Path\>: This Mode Will Allow A Client To Request The Server To Look Through Its Relative Directory For The Given File Path And If It's Currently Created Will Create A File Under That File Path. The Client Will Then Write From STDIO Their Input In Which Will Be Communicated By The Server Through Byte-Streams On A Line-By-Line Basis Until The User Inputs A EOF Character.

 - client \<IP\> \<Socket\>: This Mode Will Work By Asking The Client For A Single Line From STDIO And Will Communicate The Contents Of This Input To The Server In Which Will Simply Echo This Content (Mainly Utilized For Debugging To Ensure Clean Transactions).

The Server Also Can Be Shutdown With A CTRL + C Input; This Will Shutdown All Detached Server Threads And Will Communicate With The User The Shutdown Of The Server.

The Client Will Need To Launch The Process Of This Program With The Simple Appension Of The Mode Type In Place Of The 'server' During The Calling Of The Program. Other Than These Modes, The Server Will Tell The User Of A Invalid Transaction Mode Type And Their Client Process Will Terminate.

<img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/e75c904e-323b-449d-a9d9-4b50a4442a0d" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/e75c904e-323b-449d-a9d9-4b50a4442a0d" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/e75c904e-323b-449d-a9d9-4b50a4442a0d" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/e75c904e-323b-449d-a9d9-4b50a4442a0d" alt="Cornstarch <3" width="55" height="49">

----------------------------------------------------------------------------

<img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/06f25a05-7be6-41a6-81ee-0f746b3fa527" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/06f25a05-7be6-41a6-81ee-0f746b3fa527" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/06f25a05-7be6-41a6-81ee-0f746b3fa527" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/06f25a05-7be6-41a6-81ee-0f746b3fa527" alt="Cornstarch <3" width="55" height="49">

**Features:**

Client Requesting A Invalid Mode & Writing From STDIO Into A File Path & Reading From Server The File Wrote In:
![2024-06-0812-19-17-ezgif com-video-to-gif-converter](https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/dd7819df-1ec2-434d-a094-0c8e892ee41c)


<img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/96b54435-9884-4edd-82cb-0b2ac7bd75ef" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/96b54435-9884-4edd-82cb-0b2ac7bd75ef" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/96b54435-9884-4edd-82cb-0b2ac7bd75ef" alt="Cornstarch <3" width="55" height="49"> <img src="https://github.com/Kingerthanu/CPP_ClientServer/assets/76754592/96b54435-9884-4edd-82cb-0b2ac7bd75ef" alt="Cornstarch <3" width="55" height="49">
