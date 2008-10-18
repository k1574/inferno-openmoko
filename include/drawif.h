extern Font*		checkfont(Draw_Font*);
extern Image*		checkimage(Draw_Image*);
extern Screen*		checkscreen(Draw_Screen*);
extern Display*		checkdisplay(Draw_Display*);
extern void		freedrawdisplay(Heap*, int);
extern void		freedrawfont(Heap*, int);
extern void		freedrawimage(Heap*, int);
extern void		freedrawscreen(Heap*, int);
extern Font*		lookupfont(Draw_Font*);
extern Image*		lookupimage(const Draw_Image*);
extern Screen*		lookupscreen(Draw_Screen*);
extern Draw_Image*	mkdrawimage(Image*, Draw_Screen*, Draw_Display*, void*);
