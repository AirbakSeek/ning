# NING
  It is used for compress and extract data.
  It may just is suitable for running on large data servers.

## What is the principle?
  Any data could be decomposed like following:  
  Data_A = F1(x)F2(y)F3(z)...FN(...)  
  x,y,z could be treaded as a new Data_B.  
  Data_B also coulde be decomposed to x,y,z.  
  Data_B should be smaller than Data_A.  
  After N times decompos, Data_N is the least.  
  Data_N record the x,y,z and N.  
  NING provide F1，F2，F3 and return Data_N.  
  Like a data bank, you keep 1KB Data_N as key, and NING return you 1GB Data_A.

## How to do?
  F1 = sha256  
  F2 = Fisrt_128bit  
  F3 has not been set, but I think F3 will be used on that day always come.  
  Data more and more, sha256 will be cracked.  
  I will not say it is my target actually. :)  
  F2 is used to solve sha256 conflict,   
  the 128bit is the balance of compression rate and conflict probability.

## Future work:
1) Handle folder.  
2) Multithreading.  
3) Restore file properties.  
4) Handle F2 conflict.  
5) Add testcase.  
6) Add configure.  

## It's all for personal interest, welcome to join.
  Email: airbak@126.com  
  Weixin: airbak  
  QQ: 103916390  


