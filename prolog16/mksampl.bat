@echo off
wcc -ml -q -wx -d2 prolog16.c
wcl -ml -q -wx -ep -en -ee -d2 -Fm prosamp.c prolog16.obj
