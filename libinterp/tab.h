struct
{
	char*	name;
	int	op;
	int	terminal;
}keywds[] =
{
	"nop",		INOP,		TOKI0,
	"alt",		IALT,		TOKI3,
	"nbalt",	INBALT,		TOKI3,
	"goto",		IGOTO,		TOKI2,
	"call",		ICALL,		TOKI2,
	"frame",	IFRAME,		TOKI2,
	"spawn",	ISPAWN,		TOKI2,
	"runt",		IRUNT,		TOKI2,
	"load",		ILOAD,		TOKI3,
	"mcall",	IMCALL,		TOKI3,
	"mspawn",	IMSPAWN,	TOKI3,
	"mframe",	IMFRAME,	TOKI3,
	"ret",		IRET,		TOKI0,
	"jmp",		IJMP,		TOKI1,
	"case",		ICASE,		TOKI2,
	"exit",		IEXIT,		TOKI0,
	"new",		INEW,		TOKI2,
	"newa",		INEWA,		TOKI3,
	"newcb",	INEWCB,		TOKI1,
	"newcw",	INEWCW,		TOKI1,
	"newcf",	INEWCF,		TOKI1,
	"newcp",	INEWCP,		TOKI1,
	"newcm",	INEWCM,		TOKI2,
	"newcmp",	INEWCMP,	TOKI2,
	"send",		ISEND,		TOKI2,
	"recv",		IRECV,		TOKI2,
	"consb",	ICONSB,		TOKI2,
	"consw",	ICONSW,		TOKI2,
	"consp",	ICONSP,		TOKI2,
	"consf",	ICONSF,		TOKI2,
	"consm",	ICONSM,		TOKI3,
	"consmp",	ICONSMP,	TOKI3,
	"headb",	IHEADB,		TOKI2,
	"headw",	IHEADW,		TOKI2,
	"headp",	IHEADP,		TOKI2,
	"headf",	IHEADF,		TOKI2,
	"headm",	IHEADM,		TOKI3,
	"headmp",	IHEADMP,	TOKI3,
	"tail",		ITAIL,		TOKI2,
	"lea",		ILEA,		TOKI2,
	"indx",		IINDX,		TOKI3,
	"movp",		IMOVP,		TOKI2,
	"movm",		IMOVM,		TOKI3,
	"movmp",	IMOVMP,		TOKI3,
	"movb",		IMOVB,		TOKI2,
	"movw",		IMOVW,		TOKI2,
	"movf",		IMOVF,		TOKI2,
	"cvtbw",	ICVTBW,		TOKI2,
	"cvtwb",	ICVTWB,		TOKI2,
	"cvtfw",	ICVTFW,		TOKI2,
	"cvtwf",	ICVTWF,		TOKI2,
	"cvtca",	ICVTCA,		TOKI2,
	"cvtac",	ICVTAC,		TOKI2,
	"cvtwc",	ICVTWC,		TOKI2,
	"cvtcw",	ICVTCW,		TOKI2,
	"cvtfc",	ICVTFC,		TOKI2,
	"cvtcf",	ICVTCF,		TOKI2,
	"addb",		IADDB,		TOKI3,
	"addw",		IADDW,		TOKI3,
	"addf",		IADDF,		TOKI3,
	"subb",		ISUBB,		TOKI3,
	"subw",		ISUBW,		TOKI3,
	"subf",		ISUBF,		TOKI3,
	"mulb",		IMULB,		TOKI3,
	"mulw",		IMULW,		TOKI3,
	"mulf",		IMULF,		TOKI3,
	"divb",		IDIVB,		TOKI3,
	"divw",		IDIVW,		TOKI3,
	"divf",		IDIVF,		TOKI3,
	"modw",		IMODW,		TOKI3,
	"modb",		IMODB,		TOKI3,
	"andb",		IANDB,		TOKI3,
	"andw",		IANDW,		TOKI3,
	"orb",		IORB,		TOKI3,
	"orw",		IORW,		TOKI3,
	"xorb",		IXORB,		TOKI3,
	"xorw",		IXORW,		TOKI3,
	"shlb",		ISHLB,		TOKI3,
	"shlw",		ISHLW,		TOKI3,
	"shrb",		ISHRB,		TOKI3,
	"shrw",		ISHRW,		TOKI3,
	"insc",		IINSC,		TOKI3,
	"indc",		IINDC,		TOKI3,
	"addc",		IADDC,		TOKI3,
	"lenc",		ILENC,		TOKI2,
	"lena",		ILENA,		TOKI2,
	"lenl",		ILENL,		TOKI2,
	"beqb",		IBEQB,		TOKI3,
	"bneb",		IBNEB,		TOKI3,
	"bltb",		IBLTB,		TOKI3,
	"bleb",		IBLEB,		TOKI3,
	"bgtb",		IBGTB,		TOKI3,
	"bgeb",		IBGEB,		TOKI3,
	"beqw",		IBEQW,		TOKI3,
	"bnew",		IBNEW,		TOKI3,
	"bltw",		IBLTW,		TOKI3,
	"blew",		IBLEW,		TOKI3,
	"bgtw",		IBGTW,		TOKI3,
	"bgew",		IBGEW,		TOKI3,
	"beqf",		IBEQF,		TOKI3,
	"bnef",		IBNEF,		TOKI3,
	"bltf",		IBLTF,		TOKI3,
	"blef",		IBLEF,		TOKI3,
	"bgtf",		IBGTF,		TOKI3,
	"bgef",		IBGEF,		TOKI3,
	"beqc",		IBEQC,		TOKI3,
	"bnec",		IBNEC,		TOKI3,
	"bltc",		IBLTC,		TOKI3,
	"blec",		IBLEC,		TOKI3,
	"bgtc",		IBGTC,		TOKI3,
	"bgec",		IBGEC,		TOKI3,
	"slicea",	ISLICEA,	TOKI3,
	"slicela",	ISLICELA,	TOKI3,
	"slicec",	ISLICEC,	TOKI3,
	"indw",		IINDW,		TOKI3,
	"indf",		IINDF,		TOKI3,
	"indb",		IINDB,		TOKI3,
	"negf",		INEGF,		TOKI2,
	"movl",		IMOVL,		TOKI2,
	"addl",		IADDL,		TOKI3,
	"subl",		ISUBL,		TOKI3,
	"divl",		IDIVL,		TOKI3,
	"modl",		IMODL,		TOKI3,
	"mull",		IMULL,		TOKI3,
	"andl",		IANDL,		TOKI3,
	"orl",		IORL,		TOKI3,
	"xorl",		IXORL,		TOKI3,
	"shll",		ISHLL,		TOKI3,
	"shrl",		ISHRL,		TOKI3,
	"bnel",		IBNEL,		TOKI3,
	"bltl",		IBLTL,		TOKI3,
	"blel",		IBLEL,		TOKI3,
	"bgtl",		IBGTL,		TOKI3,
	"bgel",		IBGEL,		TOKI3,
	"beql",		IBEQL,		TOKI3,
	"cvtlf",	ICVTLF,		TOKI2,
	"cvtfl",	ICVTFL,		TOKI2,
	"cvtlw",	ICVTLW,		TOKI2,
	"cvtwl",	ICVTWL,		TOKI2,
	"cvtlc",	ICVTLC,		TOKI2,
	"cvtcl",	ICVTCL,		TOKI2,
	"headl",	IHEADL,		TOKI2,
	"consl",	ICONSL,		TOKI2,
	"newcl",	INEWCL,		TOKI1,
	"casec",	ICASEC,		TOKI2,
	"indl",		IINDL,		TOKI3,
	"movpc",	IMOVPC,		TOKI2,
	"tcmp",		ITCMP,		TOKI2,
	"mnewz",	IMNEWZ,		TOKI3,	/* j2d */
	"cvtrf",	ICVTRF,		TOKI2,	/* j2d */
	"cvtfr",	ICVTFR,		TOKI2,	/* j2d */
	"cvtws",	ICVTWS,		TOKI2,	/* j2d */
	"cvtsw",	ICVTSW,		TOKI2,	/* j2d */
	"lsrw",		ILSRW,		TOKI3,	/* j2d */
	"lsrl",		ILSRL,		TOKI3,	/* j2d */
	"eclr",		IECLR,		TOKI0,	/* unused */
	"newz",		INEWZ,		TOKI2,	/* j2d */
	"newaz",	INEWAZ,		TOKI3,	/* j2d */
	"raise",	IRAISE,	TOKI1,
	"casel",	ICASEL,	TOKI2,
	"mulx",	IMULX,	TOKI3,
	"divx",	IDIVX,	TOKI3,
	"cvtxx",	ICVTXX,	TOKI3,
	"mulx0",	IMULX0,	TOKI3,
	"divx0",	IDIVX0,	TOKI3,
	"cvtxx0",	ICVTXX0,	TOKI3,
	"mulx1",	IMULX1,	TOKI3,
	"divx1",	IDIVX1,	TOKI3,
	"cvtxx1",	ICVTXX1,	TOKI3,
	"cvtfx",	ICVTFX,	TOKI3,
	"cvtxf",	ICVTXF,	TOKI3,
	"expw",	IEXPW,	TOKI3,
	"expl",	IEXPL,	TOKI3,
	"expf",	IEXPF,	TOKI3,
	"self",	ISELF,	TOKI1,
	0,
};
