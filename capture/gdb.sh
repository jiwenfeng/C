#!/bin/sh
PID=`ps axu|grep main|grep -v grep|grep -v sudo|awk '{print $2}'`

gdb -p $PID

