#include <lib9.h>
#include <isa.h>
#include <interp.h>
#include <runt.h>
#include <loadermod.h>
#include <raise.h>
#include <kernel.h>

static char	Instmap[] = Loader_Inst_map;
static Type*	Tinst;
static char	Tdescmap[] = Loader_Typedesc_map;
static Type*	Tdesc;
static char	Tlinkmap[] = Loader_Link_map;
static Type*	Tlink;

void
loadermodinit(void)
{
	sysinit();
	builtinmod("$Loader", Loadermodtab, Loadermodlen);
	Tinst = dtype(freeheap, sizeof(Loader_Inst), Instmap, sizeof(Instmap), "Loader->inst");
	Tdesc = dtype(freeheap, sizeof(Loader_Typedesc), Tdescmap, sizeof(Tdescmap), "Loader->desc");
	Tlink = dtype(freeheap, sizeof(Loader_Link), Tlinkmap, sizeof(Tlinkmap), "Loader->link");
}

static void
brunpatch(Loader_Inst *ip, Module *m)
{
	switch(ip->op) {
	case ICALL:
	case IJMP:
	case IBEQW:
	case IBNEW:
	case IBLTW:
	case IBLEW:
	case IBGTW:
	case IBGEW:
	case IBEQB:
	case IBNEB:
	case IBLTB:
	case IBLEB:
	case IBGTB:
	case IBGEB:
	case IBEQF:
	case IBNEF:
	case IBLTF:
	case IBLEF:
	case IBGTF:
	case IBGEF:
	case IBEQC:
	case IBNEC:
	case IBLTC:
	case IBLEC:
	case IBGTC:
	case IBGEC:
	case IBEQL:
	case IBNEL:
	case IBLTL:
	case IBLEL:
	case IBGTL:
	case IBGEL:
	case ISPAWN:
		ip->dst = (Inst*)ip->dst - m->prog;
		break;
	}
}

DISAPI(Loader_ifetch)
{
	Heap *h;
	Array *ar;
	Module *m;
	Inst *i, *ie;
	Loader_Inst *li;

	ASSIGN(*f->ret, H);

	if(f->mp == H)
		return;
	m = f->mp->m;
	if(m == H)
		return;
	if(m->compiled) {
		kwerrstr("compiled module");
		return;
	}

	h = nheap(sizeof(Array)+m->nprog*sizeof(Loader_Inst));
	h->t = &Tarray;
	h->t->ref++;
	ar = H2D(Array*, h);
	ar->t = Tinst;
	Tinst->ref++;
	ar->len = m->nprog;
	ar->root = (Array*)H;
	ar->data = (char*)ar+sizeof(Array);

	li = (Loader_Inst*)ar->data;
	i = m->prog;
	ie = i + m->nprog;
	while(i < ie) {
		li->op = i->op;
		li->addr = i->add;
		li->src = i->s.imm;
		li->dst = i->d.imm;
		li->mid = i->reg;
		if(UDST(i->add) == AIMM)
			brunpatch(li, m);
		li++;
		i++;
	}

	*f->ret = ar;
}

DISAPI(Loader_link)
{
	Link *p;
	Heap *h;
	Type **t;
	int nlink;
	Module *m;
	Array *ar;
	Loader_Link *ll;

	ASSIGN(*f->ret, H);

	if(f->mp == H)
		return;
	m = f->mp->m;
	if(m == H)
		return;

	nlink = 0;
	for(p = m->ext; p->name; p++)
		nlink++;

	h = nheap(sizeof(Array)+nlink*sizeof(Loader_Link));
	h->t = &Tarray;
	h->t->ref++;
	ar = H2D(Array*, h);
	ar->t = Tlink;
	Tlink->ref++;
	ar->len = nlink;
	ar->root = (Array*)H;
	ar->data = (char*)ar+sizeof(Array);

	ll = (Loader_Link*)ar->data + nlink;
	for(p = m->ext; p->name; p++) {
		ll--;
		ll->name = c2string(p->name, strlen(p->name));
		ll->sig = p->sig;
		if(m->prog == nil) {
			ll->pc = -1;
			ll->tdesc = -1;
		} else {
			ll->pc = p->u.pc - m->prog;
			ll->tdesc = 0;
			for(t = m->type; *t != p->frame; t++)
				ll->tdesc++;
		}
	}

	*f->ret = ar;
}

DISAPI(Loader_tdesc)
{
	int i;
	Heap *h;
	Type *t;
	Array *ar;
	Module *m;
	Loader_Typedesc *lt;

	ASSIGN(*f->ret, H);

	if(f->mp == H)
		return;
	m = f->mp->m;
	if(m == H)
		return;

	h = nheap(sizeof(Array)+m->ntype*sizeof(Loader_Typedesc));
	h->t = &Tarray;
	h->t->ref++;
	ar = H2D(Array*, h);
	ar->t = Tdesc;
	Tdesc->ref++;
	ar->len = m->ntype;
	ar->root = (Array *)H;
	ar->data = (char*)ar+sizeof(Array);

	lt = (Loader_Typedesc*)ar->data;
	for(i = 0; i < m->ntype; i++) {
		t = m->type[i];
		lt->size = t->size;
		lt->map = (Array*)H;
		if(t->np != 0)
			lt->map = mem2array(t->map, t->np);
		lt++;
	}

	*f->ret = ar;
}

DISAPI(Loader_newmod)
{
	Module *m;
	Array *ia;
	Modlink *ml;
	Inst *i, *ie;
	Loader_Inst *li;

	ASSIGN(*f->ret, H);

	if(f->inst == H || f->data == H) {
		kwerrstr("nil parameters");
		return;
	}
	if(f->nlink < 0) {
		kwerrstr("bad nlink");
		return;
	}

	m = (Module *)malloc(sizeof(Module));
	if(m == nil) {
		kwerrstr(exNomem);
		return;
	}
	m->origmp = (char*)H;
	m->ref = 1;
	/*m->ss = f->ss;*/
	m->name = strdup(string2c(f->name));
	m->path = strdup(m->name);
	m->ntype = 1;
	m->type = (Type**)malloc(sizeof(Type*));
	if(m->name == nil || m->path == nil || m->type == nil) {
		kwerrstr(exNomem);
		goto bad;
	}
	m->origmp = (char*)f->data;
	ADDREF(f->data);
	Setmark(D2H(f->data));
	m->type[0] = D2H(f->data)->t;
	D2H(f->data)->t->ref++;

	ia = f->inst;
	m->nprog = ia->len;
	m->prog = (Inst*)malloc(m->nprog*sizeof(Inst));
	if(m->prog == nil)
		goto bad;
	i = m->prog;
	ie = i + m->nprog;
	li = (Loader_Inst*)ia->data;
	while(i < ie) {
		i->op = li->op;
		i->add = li->addr;
		i->reg = li->mid;
		i->s.imm = li->src;
		i->d.imm = li->dst;
		if(brpatch(i, m) == 0) {
			kwerrstr("bad branch addr");
			goto bad;
		}
		i++;
		li++;
	}
	m->entryt = nil;
	m->entry = m->prog;

	ml = mklinkmod(m, f->nlink);
	ml->MP = m->origmp;
	m->origmp = (char*)H;
	m->pctab = nil;
	*f->ret = ml;
	return;
bad:
	ASSIGN(m->origmp, H);
	freemod(m);
}

DISAPI(Loader_tnew)
{
	int mem;
	Module *m;
	Type *t, **nt;
	Array *ar, az;

	*f->ret = -1;
	if(f->mp == H)
		return;
	m = f->mp->m;
	if(m == H)
		return;
	if(m->origmp != H){
		kwerrstr("need newmod");
		return;
	}

	ar = f->map;
	if(ar == H) {
		ar = &az;
		ar->len = 0;
		ar->data = nil;
	}

	t = dtype(freeheap, f->size, ar->data, ar->len, "(Loader_tnew)");
	if(t == nil)
		return;

	mem = (m->ntype+1)*sizeof(Type*);
	if(msize(m->type) > mem) {
		*f->ret = m->ntype;
		m->type[m->ntype++] = t;
		return;
	}
	nt = (Type**)realloc(m->type, mem);
	if(nt == nil) {
		kwerrstr(exNomem);
		return;
	}
	m->type = nt;
	f->mp->type = nt;
	*f->ret = m->ntype;
	m->type[m->ntype++] = t;
}

DISAPI(Loader_ext)
{
	Modl *l;
	Module *m;
	Modlink *ml;

	*f->ret = -1;
	if(f->mp == H) {
		kwerrstr("nil mp");
		return;
	}
	ml = f->mp;
	m = ml->m;
	if(f->tdesc < 0 || f->tdesc >= m->ntype) {
		kwerrstr("bad tdesc");
		return;
	}
	if(f->pc < 0 || f->pc >= m->nprog) {
		kwerrstr("bad pc");
		return;
	}
	if(f->idx < 0 || f->idx >= ml->nlinks) {
		kwerrstr("bad idx");
		return;
	}
	l = &ml->links[f->idx];
	l->u.pc = m->prog + f->pc;
	l->frame = m->type[f->tdesc];
	*f->ret = 0;
}

DISAPI(Loader_dnew)
{
	Heap *h;
	Array *ar, az;
	Type *t;

        *f->ret = (Loader_Niladt*)H;
        if(f->map == H)
                return;
        ar = f->map;
        if(ar == H) {
                ar = &az;
                ar->len = 0;
                ar->data = nil;
        }
        t = dtype(freeheap, f->size, ar->data, ar->len, "(Loader_dnew)");
        if(t == nil) {
                kwerrstr(exNomem);
                return;
        }

	h=heapz(t);
	if(h == nil) {
		freetype(t);
		kwerrstr(exNomem);
		return;
        }

	*f->ret=H2D(Loader_Niladt*, h);
}

DISAPI(Loader_compile)
{
#if JIT
	Module *m;
	*f->ret = -1;
	if(f->mp == H) {
		kwerrstr("nil mp");
		return;
	}
	m = f->mp->m;
	if(m->compiled) {
		kwerrstr("compiled module");
		return;
	}
	*f->ret = 0;
	m->origmp = f->mp->MP;
	if(cflag || f->flag)
	if(compile(m, m->nprog, f->mp)) {
		f->mp->prog = m->prog;
		f->mp->compiled = 1;
	} else
		*f->ret = -1;
	m->origmp = (char*)H;
#else
        panic("Loader.compile()");
#endif
}
