tcc -I..\include -I..\Nt\386\include -I..\lib9 ..\libinterp\decgen.c ..\lib9\print.c ..\lib9\vfprint.c ..\lib9\dofmt.c  ..\lib9\fmtfd.c ..\lib9\rune.c ..\lib9\utflen.c ..\lib9\utfnlen.c ..\lib9\fmt.c ..\lib9\errfmt.c ..\lib9\rerrstr.c ..\lib9\fmtlock.c ..\lib9\utfecpy.c ..\lib9\errstr-Nt.c ..\lib9\vseprint.c ..\lib9\snprint.c ..\lib9\vsnprint.c -run > ..\libinterp\dec.c