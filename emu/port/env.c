#include <dat.h>
#include <fns.h>
#include <error.h>

Egrp*
newegrp(void)
{
	Egrp	*e;

	e = (Egrp*)smalloc(sizeof(Egrp));
	if (e == nil)
		error(Enomem);
	e->r.ref = 1;
	return e;
}

void
closeegrp(Egrp *e)
{
	Evalue *el, *nl;

	if(e == nil || decref(&e->r) != 0)
		return;
	for (el = e->entries; el != nil; el = nl) {
		free(el->var);
		if (el->val)
			free(el->val);
		nl = el->next;
		free(el);
	}
	free(e);
}

void
egrpcpy(Egrp *to, Egrp *from)
{
	Evalue *e, *ne, **last;

	last = &to->entries;
	qlock(&from->l);
	for (e = from->entries; e != nil; e = e->next) {
		ne = (Evalue*)smalloc(sizeof(Evalue));
		ne->var = (char*)smalloc(strlen(e->var)+1); /*TODO: strdup*/
		strcpy(ne->var, e->var);
		if (e->val) {
			ne->val = (char*)smalloc(e->len);
			memmove(ne->val, e->val, e->len);
			ne->len = e->len;
		}
		ne->qid.path = ++to->path;
		*last = ne;
		last = &ne->next;
	}
	qunlock(&from->l);
}

void
ksetenv(const char *var, const char *val, int conf)
{
	Chan *c;
	char buf[2*KNAMELEN];

	USED(conf);
	snprint(buf, sizeof(buf), "#e/%s", var);
	if(waserror())
		return;
	c = namec(buf, Acreate, OWRITE, 0600);
	poperror();
	if(!waserror()){
		if(!waserror()){
			devtab[c->type]->write(c, val, strlen(val), 0);
			poperror();
		}
		poperror();
	}
	cclose(c);
}
