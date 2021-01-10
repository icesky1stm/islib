# islib
is c Loose coupling general lib
islib是一个积累的不断更新的纯C的松耦合的基础组件库，有如下两个原则:
1. 任何功能直接松耦合，互相无依赖. 库中的任何一个.c和.h构成的一个组件都可以单独使用.
2. 每个组件都有自己的版本， 所有的组件共同构成了islib的版本.

因此，如果要单独使用某一个组件如istime, 只需要拷贝istime.c和istime.h即可。
如果想把islib整体作为一个基础组件来使用，则可以编译成islib来进行链接使用.

由于islib是一个不断迭代的lib库，因此使用了ubuntu和jetbrains的版本管理方式，具体表现为X.Y.Z.A
其中:
    - X , 表示当前年份 - 2000
    - Y , 表示当前月份 
    - Z , 表示排序
    - A , 不一定有，如有则表现为alpha, beta, lastern，RC1, RC2等，一般是内部版本，表示同一个版本的发布状态。
    
    
TODO:
    - islog, 增加一个高可用的、极简的日志库. 学习一下easylogger和zlog
    - ismap, 移植xipHashmap.
    - islist, 移植xipArrayList.
    - isipc, 增加ipc操作的处理.
    - isatom, 原子操作，参考redis的atomic.h
    - 
    