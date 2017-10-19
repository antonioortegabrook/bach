#include "bach.h"

#define BACH_MAX_ATTRS 1024 
#ifdef BACH_MAX
#include "ext.h"
#include "ext_obex.h"
#endif

#ifdef BACH_JUCE
#include "bach_jucewrapper.h"
#endif

#include "llll_commonsyms.h"

t_llllobj_common_symbols_table *_llllobj_common_symbols;
t_symbol **_llllobj_attributes;

char _open_parentheses[3] = {'(', '[', '{'};
char _closed_parentheses[3] = {')', ']', '}'};
t_symbol *_open_parentheses_sym[3];
t_symbol *_closed_parentheses_sym[3];

t_max_err llllobj_common_symbols_init(void)
{
	t_symbol **this_attr = _llllobj_attributes = (t_symbol **) sysmem_newptr(BACH_MAX_ATTRS * sizeof(t_symbol *));

	*this_attr++ = gensym("accidentalsfont");
	*this_attr++ = gensym("accidentalsgraphic");
	*this_attr++ = gensym("accidentalspreferences");
	*this_attr++ = gensym("activeslot");
	*this_attr++ = gensym("additionalstartpad");
	*this_attr++ = gensym("admitcolswap");
	*this_attr++ = gensym("admitdottedtuplets");
	*this_attr++ = gensym("algorithm");
	*this_attr++ = gensym("align");
	*this_attr++ = gensym("allowbarlinelock");
	*this_attr++ = gensym("allowbeaming");
	*this_attr++ = gensym("allowcopypaste");
	*this_attr++ = gensym("allowglissandi");
	*this_attr++ = gensym("allowlock");
	*this_attr++ = gensym("allowmute");
	*this_attr++ = gensym("allowsolo");
	*this_attr++ = gensym("alternate");
	*this_attr++ = gensym("approxwidth");
	*this_attr++ = gensym("arcconsistency");
	*this_attr++ = gensym("attrname");
	*this_attr++ = gensym("attrs");
	*this_attr++ = gensym("auto");
	*this_attr++ = gensym("autoceil");
	*this_attr++ = gensym("autoclear");
	*this_attr++ = gensym("autoclearrollmarkers");
	*this_attr++ = gensym("autocolor");
	*this_attr++ = gensym("autocorrect");
	*this_attr++ = gensym("autojump");
	*this_attr++ = gensym("automessage");
	*this_attr++ = gensym("autoparserhythm");
	*this_attr++ = gensym("autoreduce");
	*this_attr++ = gensym("autosend");
	*this_attr++ = gensym("autosize");
	*this_attr++ = gensym("backgroundslotfontsize");
	*this_attr++ = gensym("backgroundslots");
	*this_attr++ = gensym("backtick");
	*this_attr++ = gensym("base");
	*this_attr++ = gensym("beamgracesequences");
	*this_attr++ = gensym("bgslotfontsize");
	*this_attr++ = gensym("bgslots");
	*this_attr++ = gensym("bgslottextshift");
	*this_attr++ = gensym("bgslotzoom");
	*this_attr++ = gensym("boxedgetiedurthresh");
	*this_attr++ = gensym("breakpointshavenoteheads");
	*this_attr++ = gensym("breakpointshavevelocity");
	*this_attr++ = gensym("breakrootlevelbeam");
	*this_attr++ = gensym("cautionaryaccidentals");
	*this_attr++ = gensym("cautionaryaccidentalsdecay");
	*this_attr++ = gensym("cautionaryaccidentalsremind");
	*this_attr++ = gensym("changecentstomeasuresbydragging");
	*this_attr++ = gensym("check");
	*this_attr++ = gensym("chunksize");
	*this_attr++ = gensym("circlelinewidth");
	*this_attr++ = gensym("circular");
	*this_attr++ = gensym("clefs");
	*this_attr++ = gensym("clustersize");
	*this_attr++ = gensym("coloredampli");
	*this_attr++ = gensym("constraintbeamsinspaces");
	*this_attr++ = gensym("continuouslyoutputbangifchanged");
	*this_attr++ = gensym("count");
	*this_attr++ = gensym("dblclicksendsvalues");
	*this_attr++ = gensym("debugbpm");
	*this_attr++ = gensym("decaytime");
	*this_attr++ = gensym("den2");
	*this_attr++ = gensym("den3");
	*this_attr++ = gensym("denns");
	*this_attr++ = gensym("depthreject");
	*this_attr++ = gensym("discardgrace");
	*this_attr++ = gensym("discardgracerests");
	*this_attr++ = gensym("discardsegmentsunder");
	*this_attr++ = gensym("discardsegmentsunderrelweight");
	*this_attr++ = gensym("done");
	*this_attr++ = gensym("drawbarlinesacrossstaves");
	*this_attr++ = gensym("durationlinewidth");
	*this_attr++ = gensym("durthresh");
	*this_attr++ = gensym("embed");
	*this_attr++ = gensym("enclosure");
	*this_attr++ = gensym("enharmonictable");
	*this_attr++ = gensym("error");
	*this_attr++ = gensym("evennessaffectsweight");
	*this_attr++ = gensym("extendbeamsoverrests");
	*this_attr++ = gensym("firsteleminllllisllllname");
	*this_attr++ = gensym("gainmax");
	*this_attr++ = gensym("gainmin");
	*this_attr++ = gensym("gracedeletethresh");
	*this_attr++ = gensym("gracedur");
	*this_attr++ = gensym("gracemaxperc");
	*this_attr++ = gensym("graphtype");
	*this_attr++ = gensym("grid");
	*this_attr++ = gensym("gridperiodms");
	*this_attr++ = gensym("gridrstep");
	*this_attr++ = gensym("gridthetastep");
	*this_attr++ = gensym("gridtype");
	*this_attr++ = gensym("gridxstep");
	*this_attr++ = gensym("gridystep");
	*this_attr++ = gensym("group");
	*this_attr++ = gensym("harmchange");
	*this_attr++ = gensym("harmchangefilterwinsize");
	*this_attr++ = gensym("hcclustersize");
	*this_attr++ = gensym("hchopsize");
	*this_attr++ = gensym("hcwinsize");
	*this_attr++ = gensym("hidetupletshapeonbeams");
	*this_attr++ = gensym("hidevoices");
	*this_attr++ = gensym("highlightplay");
	*this_attr++ = gensym("hinset");
	*this_attr++ = gensym("hint");
	*this_attr++ = gensym("hyperlinkchars");
	*this_attr++ = gensym("idxreject");
	*this_attr++ = gensym("index");
	*this_attr++ = gensym("ins");
	*this_attr++ = gensym("inset");
	*this_attr++ = gensym("intterruptchars");
	*this_attr++ = gensym("inv");
	*this_attr++ = gensym("inwrap");
	*this_attr++ = gensym("iterationmode");
	*this_attr++ = gensym("jointuplets");
	*this_attr++ = gensym("keep");
	*this_attr++ = gensym("keepselectioniflostfocus");
	*this_attr++ = gensym("kend");
	*this_attr++ = gensym("keys");
	*this_attr++ = gensym("kstart");
	*this_attr++ = gensym("labelsxstep");
	*this_attr++ = gensym("labelsystep");
	*this_attr++ = gensym("legend");
	*this_attr++ = gensym("legendfontsize");
	*this_attr++ = gensym("levels");
	*this_attr++ = gensym("leveltobeam");
	*this_attr++ = gensym("linearedit");
	*this_attr++ = gensym("linewidth");
	*this_attr++ = gensym("linklyricstoslot");
	*this_attr++ = gensym("linknotecolortoslot");
	*this_attr++ = gensym("linknoteheadadjusttoslot");
	*this_attr++ = gensym("linknoteheadfonttoslot");
	*this_attr++ = gensym("linknoteheadtoslot");
	*this_attr++ = gensym("linknotesizetoslot");
	*this_attr++ = gensym("llllsymbol");
	*this_attr++ = gensym("longsegments");
	*this_attr++ = gensym("loop");
	*this_attr++ = gensym("lyricsaffectspacing");
	*this_attr++ = gensym("lyricsalignment");
	*this_attr++ = gensym("lyricsfontsize");
	*this_attr++ = gensym("lyricsvadj");
	*this_attr++ = gensym("makeorthogonal");
	*this_attr++ = gensym("maketreecompatiblewithts");
	*this_attr++ = gensym("makeunitary");
	*this_attr++ = gensym("markersfontsize");
	*this_attr++ = gensym("masking");
	*this_attr++ = gensym("maskingext");
	*this_attr++ = gensym("maskingextover");
	*this_attr++ = gensym("matchtype");
	*this_attr++ = gensym("maxamplirhythmoscope");
	*this_attr++ = gensym("maxbeamdeltay");
	*this_attr++ = gensym("maxbeamslope");
	*this_attr++ = gensym("maxbpm");
	*this_attr++ = gensym("maxcoeffrandvect");
	*this_attr++ = gensym("maxcount");
	*this_attr++ = gensym("maxdecimals");
	*this_attr++ = gensym("maxdepth");
	*this_attr++ = gensym("maxdiv");
	*this_attr++ = gensym("maxdots");
	*this_attr++ = gensym("maxidx");
	*this_attr++ = gensym("maximum");
	*this_attr++ = gensym("maxiterations");
	*this_attr++ = gensym("maxlevel");
	*this_attr++ = gensym("maxrestfloatsteps");
	*this_attr++ = gensym("maxt");
	*this_attr++ = gensym("maxtempo");
	*this_attr++ = gensym("maxtheta");
	*this_attr++ = gensym("maxtime");
	*this_attr++ = gensym("maxundosteps");
	*this_attr++ = gensym("maxvars");
	*this_attr++ = gensym("maxwaveampli");
	*this_attr++ = gensym("maxx");
	*this_attr++ = gensym("maxy");
	*this_attr++ = gensym("measurenumberfontsize");
	*this_attr++ = gensym("measurenumberoffset");
	*this_attr++ = gensym("mergingpolicy");
	*this_attr++ = gensym("middlecoctave");
	*this_attr++ = gensym("midichannels");
	*this_attr++ = gensym("minampli");
	*this_attr++ = gensym("minbpm");
	*this_attr++ = gensym("mindepth");
	*this_attr++ = gensym("minidx");
	*this_attr++ = gensym("minimalunits");
	*this_attr++ = gensym("minimum");
	*this_attr++ = gensym("minlevel");
	*this_attr++ = gensym("minmeaswidth");
	*this_attr++ = gensym("minmeaswidthpersymunit");
	*this_attr++ = gensym("minpeakamp");
	*this_attr++ = gensym("minpeakrelamp");
	*this_attr++ = gensym("mint");
	*this_attr++ = gensym("mintempo");
	*this_attr++ = gensym("mintheta");
	*this_attr++ = gensym("minx");
	*this_attr++ = gensym("miny");
	*this_attr++ = gensym("mixingmode");
	*this_attr++ = gensym("mode");
	*this_attr++ = gensym("modulo");
	*this_attr++ = gensym("more");
	*this_attr++ = gensym("mousehover");
	*this_attr++ = gensym("names");
	*this_attr++ = gensym("nils");
	*this_attr++ = gensym("nodeconsistency");
	*this_attr++ = gensym("nonantialiasedstafflines");
	*this_attr++ = gensym("nonseq");
	*this_attr++ = gensym("normalize");
	*this_attr++ = gensym("notationfont");
	*this_attr++ = gensym("notenamesstyle");
	*this_attr++ = gensym("notificationsformessages");
	*this_attr++ = gensym("notifyopenslot");
	*this_attr++ = gensym("nullmode");
	*this_attr++ = gensym("numgridsubdivisions");
	*this_attr++ = gensym("numhistorywins");
	*this_attr++ = gensym("numiter");
	*this_attr++ = gensym("numpeaks");
	*this_attr++ = gensym("numpoints");
	*this_attr++ = gensym("numvoices");
	*this_attr++ = gensym("onlyclickonleaves");
	*this_attr++ = gensym("op");
	*this_attr++ = gensym("order");
	*this_attr++ = gensym("out");
	*this_attr++ = gensym("outout");
	*this_attr++ = gensym("outputllllnames");
	*this_attr++ = gensym("outputmode");
	*this_attr++ = gensym("outputslotnames");
	*this_attr++ = gensym("outputtiesindurationtree");
	*this_attr++ = gensym("outputtrees");
	*this_attr++ = gensym("outwrap");
	*this_attr++ = gensym("overlap");
	*this_attr++ = gensym("oversampling");
	*this_attr++ = gensym("pagenumberoffset");
	*this_attr++ = gensym("parallel");
	*this_attr++ = gensym("partial");
	*this_attr++ = gensym("peakshape");
	*this_attr++ = gensym("periodiconce");
	*this_attr++ = gensym("phase");
	*this_attr++ = gensym("pitcheditrange");
	*this_attr++ = gensym("pitchthreshold1");
	*this_attr++ = gensym("pitchthreshold2");
	*this_attr++ = gensym("playheaddragmode");
	*this_attr++ = gensym("playmarkers");
	*this_attr++ = gensym("playmode");
	*this_attr++ = gensym("playpartialnotes");
	*this_attr++ = gensym("playpollthrottle");
	*this_attr++ = gensym("playstep");
	*this_attr++ = gensym("playtiedelementsseparately");
	*this_attr++ = gensym("playwhenediting");
	*this_attr++ = gensym("pointlinewidth");
	*this_attr++ = gensym("pointradius");
	*this_attr++ = gensym("points");
	*this_attr++ = gensym("polar");
	*this_attr++ = gensym("popupmenuslots");
	*this_attr++ = gensym("preventedit");
	*this_attr++ = gensym("ptamp");
	*this_attr++ = gensym("ptamplow");
	*this_attr++ = gensym("ptamprel");
	*this_attr++ = gensym("ptattack");
	*this_attr++ = gensym("ptfiltersecpeaks");
	*this_attr++ = gensym("ptfreqratio");
	*this_attr++ = gensym("ptlowcut");
	*this_attr++ = gensym("ptphasedelta");
	*this_attr++ = gensym("ptrelease");
	*this_attr++ = gensym("ptvoices");
	*this_attr++ = gensym("quantizationpriority");
	*this_attr++ = gensym("random");
	*this_attr++ = gensym("realtime");
	*this_attr++ = gensym("recursionmode");
	*this_attr++ = gensym("recursive");
	*this_attr++ = gensym("reducetreeidlelevels");
	*this_attr++ = gensym("restswithinbeaming");
	*this_attr++ = gensym("reversegracedirection");
	*this_attr++ = gensym("rightclickslot");
	*this_attr++ = gensym("rounded");
	*this_attr++ = gensym("ruler");
	*this_attr++ = gensym("rulerlabels");
	*this_attr++ = gensym("rulerlabelsfontsize");
	*this_attr++ = gensym("rulermode");
	*this_attr++ = gensym("samplingrate");
	*this_attr++ = gensym("saveleveltypes");
	*this_attr++ = gensym("scalarmode");
	*this_attr++ = gensym("senddoneafterpaint");
	*this_attr++ = gensym("senddoneatstartup");
	*this_attr++ = gensym("sendnamesthroughdump");
	*this_attr++ = gensym("sendto");
	*this_attr++ = gensym("separation");
	*this_attr++ = gensym("set");
	*this_attr++ = gensym("shift");
	*this_attr++ = gensym("showaccidentalspreferences");
	*this_attr++ = gensym("showaccidentalstiepreferences");
	*this_attr++ = gensym("showarticulations");
	*this_attr++ = gensym("showarticulationsextensions");
	*this_attr++ = gensym("showauxiliarystems");
	*this_attr++ = gensym("showbarlinelocks");
	*this_attr++ = gensym("showbarlines");
	*this_attr++ = gensym("showdurations");
	*this_attr++ = gensym("showfocus");
	*this_attr++ = gensym("showgroups");
	*this_attr++ = gensym("showhand");
	*this_attr++ = gensym("showlabels");
	*this_attr++ = gensym("showledgerlines");
	*this_attr++ = gensym("showlockcolor");
	*this_attr++ = gensym("showloop");
	*this_attr++ = gensym("showlyrics");
	*this_attr++ = gensym("showlyricswordextensions");
	*this_attr++ = gensym("showmarkers");
	*this_attr++ = gensym("showmeasurenumbers");
	*this_attr++ = gensym("showmode");
	*this_attr++ = gensym("showmodulo");
	*this_attr++ = gensym("showmutecolor");
	*this_attr++ = gensym("shownotenames");
	*this_attr++ = gensym("shownumbers");
	*this_attr++ = gensym("showpagenumbers");
	*this_attr++ = gensym("showplayhead");
	*this_attr++ = gensym("showpolygon");
	*this_attr++ = gensym("showrhythmictree");
	*this_attr++ = gensym("showrhythmictreelocks");
	*this_attr++ = gensym("showrootnode");
	*this_attr++ = gensym("showscrollbar");
	*this_attr++ = gensym("showslotnumbers");
	*this_attr++ = gensym("showslurs");
	*this_attr++ = gensym("showsolocolor");
	*this_attr++ = gensym("showstems");
	*this_attr++ = gensym("showstemsforbeamedrests");
	*this_attr++ = gensym("showsynchronoustempionce");
	*this_attr++ = gensym("showtempi");
	*this_attr++ = gensym("showtimesignatures");
	*this_attr++ = gensym("showtupletspreferences");
	*this_attr++ = gensym("showunitcircle");
	*this_attr++ = gensym("showvoicenames");
	*this_attr++ = gensym("showvscrollbar");
	*this_attr++ = gensym("showxyaxis");
	*this_attr++ = gensym("simplifytuplets");
	*this_attr++ = gensym("size");
	*this_attr++ = gensym("slashgracebeams");
	*this_attr++ = gensym("slashgraceflags");
	*this_attr++ = gensym("slotminimumwindowwidth");
	*this_attr++ = gensym("slotsbgalpha");
	*this_attr++ = gensym("slotwinalpha");
	*this_attr++ = gensym("slotwinzoom");
	*this_attr++ = gensym("slursalwayssymmetrical");
	*this_attr++ = gensym("slursavoidaccidentals");
	*this_attr++ = gensym("slursshownchordwise");
	*this_attr++ = gensym("smalleventshandling");
	*this_attr++ = gensym("snapharmchange");
	*this_attr++ = gensym("snaponset");
	*this_attr++ = gensym("snappitch");
	*this_attr++ = gensym("snaptail");
	*this_attr++ = gensym("solutions");
	*this_attr++ = gensym("sortconstraints");
	*this_attr++ = gensym("sortdomains");
	*this_attr++ = gensym("spacingparameter");
	*this_attr++ = gensym("spacingproportionality");
	*this_attr++ = gensym("spacingtype");
	*this_attr++ = gensym("spacingwidth");
	*this_attr++ = gensym("spikemode");
	*this_attr++ = gensym("stafflines");
	*this_attr++ = gensym("standardsubdiv");
	*this_attr++ = gensym("subsetscardinality");
	*this_attr++ = gensym("sync");
	*this_attr++ = gensym("syncopationasymratio");
	*this_attr++ = gensym("syncopationposdurratio");
	*this_attr++ = gensym("textdecayfactor");
	*this_attr++ = gensym("textfont");
	*this_attr++ = gensym("textsize");
	*this_attr++ = gensym("tieassignspitch");
	*this_attr++ = gensym("tonedivision");
	*this_attr++ = gensym("treehandling");
	*this_attr++ = gensym("triangle");
	*this_attr++ = gensym("triggers");
	*this_attr++ = gensym("triscale");
	*this_attr++ = gensym("trytoavoidgrace");
	*this_attr++ = gensym("tupletshape");
	*this_attr++ = gensym("underline");
	*this_attr++ = gensym("unwrap");
	*this_attr++ = gensym("useacf");
	*this_attr++ = gensym("useharmony");
	*this_attr++ = gensym("useloop");
	*this_attr++ = gensym("velocities");
	*this_attr++ = gensym("velocityhandling");
	*this_attr++ = gensym("velocityslope");
	*this_attr++ = gensym("verbose");
	*this_attr++ = gensym("verboseundo");
	*this_attr++ = gensym("versionnumber");
	*this_attr++ = gensym("view");
	*this_attr++ = gensym("vinset");
	*this_attr++ = gensym("voicecoupling");
	*this_attr++ = gensym("voicenames");
	*this_attr++ = gensym("voicenamesalign");
	*this_attr++ = gensym("voicenamesfontsize");
	*this_attr++ = gensym("voicespacing");
	*this_attr++ = gensym("vzoom");
	*this_attr++ = gensym("waveshapingexp");
	*this_attr++ = gensym("winsize");
	*this_attr++ = gensym("wintype");
	*this_attr++ = gensym("writetrees");
	*this_attr++ = gensym("zeropadding");
	*this_attr++ = gensym("zfamiliesmincard");
	*this_attr++ = gensym("zoom");
	*this_attr++ = gensym("LLLL_FLOAT64_MARKER");
	*this_attr = NULL;

	
	if ((_llllobj_common_symbols = llllobj_common_symbols_gettable()))
		return MAX_ERR_NONE;
	else 
		return MAX_ERR_GENERIC;

}

t_llllobj_common_symbols_table *llllobj_common_symbols_gettable()
{
	t_llllobj_common_symbols_table *x;
	
	x = (t_llllobj_common_symbols_table *) sysmem_newptr(sizeof (t_llllobj_common_symbols_table));
	
	_open_parentheses_sym[0] = x->s_open_round_bracket = gensym("(");
	_closed_parentheses_sym[0] = x->s_closed_round_bracket = gensym(")");
	_open_parentheses_sym[1] = x->s_open_square_bracket = gensym("[");
	_closed_parentheses_sym[1] = x->s_closed_square_bracket = gensym("]");
	_open_parentheses_sym[2] = x->s_open_curly_bracket = gensym("{");
	_closed_parentheses_sym[2] = x->s_closed_curly_bracket = gensym("}");
	
	x->s_llllconnect = gensym("llll.connect");
	x->s_parse = gensym("parse");
	x->s_destroy = gensym("destroy");
	x->s_nil = gensym("nil");
	x->s_null = gensym("null");
	x->s_i = gensym("i");
	x->s_r = gensym("r");
	x->s_f = gensym("f");
	x->s_s = gensym("s");
	x->s_n = gensym("n");
	x->s_l = gensym("l");
	x->s_o = gensym("o");
    x->s_p = gensym("p");
	x->s_t = gensym("t");
	x->s_g = gensym("g");
	x->s_unknown = gensym("unknown");
	x->s_all = gensym("all");
    x->s_any = gensym("any");
    x->s_each = gensym("each");
    x->s_same = gensym("same");
    x->s_goto = gensym("goto");
	x->s_clear = gensym("clear");
	x->s_bach_lexpr = gensym("bach.lexpr");
	x->s_bach_collector = gensym("bach.collector");
	x->s_bach_llll = gensym("bach.llll");
	x->s_bach_llllelem = gensym("bach.llllelem");
	x->s_eval = gensym("eval");
	x->s_get = gensym("get");
	x->s_command = gensym("command");
	x->s__llll = gensym("_llll");
	x->s__llllelem = gensym("_llllelem");
	x->s_clone = gensym("clone");
	x->s_copy = gensym("copy");
	x->s_deparse = gensym("deparse");
	x->s_top = gensym("top");
	x->s_back = gensym("back");
	x->s_clearall = gensym("clearall");
	x->s_collection = gensym("collection");
	x->s_out = gensym("out");
	x->s_vars = gensym("vars");
	x->s_string = gensym("string");

	x->s___llll_primes__ = gensym("__llll_primes__");

	x->s_roll = gensym("roll");
	x->s_score = gensym("score");
	x->s_graphic = gensym("graphic");
	x->s_breakpoints = gensym("breakpoints");
	x->s_articulations = gensym("articulations");
    x->s_articulationinfo = gensym("articulationinfo");
    x->s_noteheadinfo = gensym("noteheadinfo");
    x->s_notehead = gensym("notehead");
    x->s_noteheads = gensym("noteheads");
    x->s_annotation = gensym("annotation");
    x->s_numparts = gensym("numparts");
    x->s_parts = gensym("parts");
	x->s_slots = gensym("slots");
	x->s_function = gensym("function");
	x->s_int = gensym("int");
	x->s_float = gensym("float");
	x->s_text = gensym("text");
	x->s_llll = gensym("llll");
	x->s_intlist = gensym("intlist");
	x->s_floatlist = gensym("floatlist");
	x->s_intmatrix = gensym("intmatrix");
	x->s_floatmatrix = gensym("floatmatrix");
	x->s_togglematrix = gensym("togglematrix");
	x->s_filelist = gensym("filelist");
	x->s_spat = gensym("spat");
	x->s_filter = gensym("filter");
	x->s_dynfilter = gensym("dynfilter");
	x->s_color = gensym("color");
	x->s_3dfunction = gensym("3dfunction");
    x->s_dynamics = gensym("dynamics");
    x->s_access = gensym("access");
    x->s_readonly = gensym("readonly");
    x->s_readandwrite = gensym("readandwrite");
	
	x->s_none = gensym("none");
	x->s_onset = gensym("onset");
	x->s_onsets = gensym("onsets");
	x->s_cent = gensym("cent");
	x->s_cents = gensym("cents");
	x->s_deltacents = gensym("deltacents");
	x->s_duration = gensym("duration");
	x->s_durations = gensym("durations");
	x->s_velocity = gensym("velocity");
	x->s_velocities = gensym("velocities");
	x->s_extra = gensym("extra");
	x->s_extras = gensym("extras");
	x->s_slotinfo = gensym("slotinfo");
	x->s_start = gensym("start");
	x->s_end = gensym("end");
	x->s_note = gensym("note");
	x->s_chord = gensym("chord");
	x->s_rest = gensym("rest");
	x->s_symduration = gensym("symduration");
	x->s_symonset = gensym("symonset");
	x->s_symtail = gensym("symtail");
	
	x->s_FFGG = gensym("FFGG");
	x->s_FFG = gensym("FFG");
	x->s_FGG = gensym("FGG");
	x->s_FF = gensym("FF");
	x->s_GG = gensym("GG");
	x->s_FG = gensym("FG");
	x->s_F = gensym("F");
	x->s_G = gensym("G");
	x->s_G15 = gensym("G15");
	x->s_F15 = gensym("F15");
	x->s_G8 = gensym("G8");
	x->s_F8 = gensym("F8");
	x->s_Soprano = gensym("Soprano");
	x->s_Mezzo = gensym("Mezzo");
	x->s_Alto = gensym("Alto");
	x->s_Tenor = gensym("Tenor");
	x->s_Barytone = gensym("Barytone");
	x->s_Bass = gensym("Bass");
	x->s_Percussion = gensym("Percussion");
	x->s_GF = gensym("GF");
	x->s_GGFF = gensym("GGFF");
	x->s_GFF = gensym("GFF");
	x->s_GGF = gensym("GGF");
    x->s_G8va = gensym("G8va");
    x->s_G8vb = gensym("G8vb");
    x->s_G15ma = gensym("G15ma");
    x->s_G15mb = gensym("G15mb");
    x->s_F8va = gensym("F8va");
    x->s_F8vb = gensym("F8vb");
    x->s_F15ma = gensym("F15ma");
    x->s_F15mb = gensym("F15mb");

	x->s_ID = gensym("ID");
	x->s_domain = gensym("domain");
	x->s_length = gensym("length");
    x->s_insertvoice = gensym("insertvoice");
    x->s_deletevoice = gensym("deletevoice");
	x->s_addchord = gensym("addchord");
	x->s_gluechord = gensym("gluechord");
	x->s_midichannels = gensym("midichannels");
	x->s_list = gensym("list");
	x->s_measureinfo = gensym("measureinfo");
	x->s_measure = gensym("measure");
	x->s_measures = gensym("measures");
	x->s_division = gensym("division");
    x->s_subdivision = gensym("subdivision");
	x->s_tie = gensym("tie");
	x->s_ties = gensym("ties");
	x->s_score2roll = gensym("score2roll");
	x->s_cursor = gensym("cursor");
	x->s_commands = gensym("commands");
	x->s_play = gensym("play");
	x->s_stop = gensym("stop");
    x->s_pause = gensym("pause");
	x->s_clefs = gensym("clefs");
	x->s_keys = gensym("keys");
	x->s_markers = gensym("markers");
	x->s_marker = gensym("marker");
	x->s_groups = gensym("groups");
    x->s_addtempo = gensym("addtempo");
    x->s_addmeasure = gensym("addmeasure");
	x->s_addmeasures = gensym("addmeasures");
    x->s_appendmeasure = gensym("appendmeasure");
    x->s_appendmeasures = gensym("appendmeasures");
    x->s_insertmeasure = gensym("insertmeasure");
    x->s_insertmeasures = gensym("insertmeasures");
	x->s_addchords = gensym("addchords");
	x->s_slope = gensym("slope");
	x->s_representation = gensym("representation");
	x->s_ysnap = gensym("ysnap");
	x->s_zsnap = gensym("zsnap");
	x->s_body = gensym("body");
	x->s_name = gensym("name");
	x->s_type = gensym("type");
	x->s_key = gensym("key");
	x->s_range = gensym("range");
    x->s_pixel = gensym("pixel");
    x->s_time = gensym("time");
    x->s_timepoint = gensym("timepoint");
	x->s_voicepixelpos = gensym("voicepixelpos");
	x->s_timeatpixel = gensym("timeatpixel");
	x->s_width = gensym("width");
	x->s_temporal = gensym("temporal");
    x->s_relative = gensym("relative");
    x->s_temporalmode = gensym("temporalmode");
    x->s_extend = gensym("extend");
    x->s_milliseconds = gensym("milliseconds");
    x->s_timepoints = gensym("timepoints");
	x->s_abr_none_abr = gensym("<none>");
	x->s_legato = gensym("legato");
    x->s_legatotrim = gensym("legatotrim");
    x->s_legatoextend = gensym("legatoextend");
    x->s_glissando = gensym("glissando");
    x->s_glissandotrim = gensym("glissandotrim");
    x->s_glissandoextend = gensym("glissandoextend");
	x->s_voicenames = gensym("voicenames");
	x->s_dyn = gensym("dyn");
	x->s_dynamic = gensym("dynamic");
	x->s_lambda = gensym("lambda");
	x->s_tail = gensym("tail");
	x->s_eraseslot = gensym("eraseslot");
	x->s_changeslotvalue = gensym("changeslotvalue");
    x->s_changeslotitem = gensym("changeslotitem");
    x->s_addslotitem = gensym("addslotitem");
    x->s_appendslotitem = gensym("appendslotitem");
    x->s_prependslotitem = gensym("prependslotitem");
    x->s_insertslotitem = gensym("insertslotitem");
    x->s_deleteslotitem = gensym("deleteslotitem");
	x->s_addslot = gensym("addslot");
	x->s_addbreakpoint = gensym("addbreakpoint");
	x->s_erasebreakpoints = gensym("erasebreakpoints");
	x->s_widthfactor = gensym("widthfactor");
	x->s_currentchord = gensym("currentchord");
	x->s_Auto = gensym("Auto");
	x->s_auto = gensym("auto");
	x->s_domainslope = gensym("domainslope");
	x->s_label = gensym("label");
	x->s_shownumber = gensym("shownumber");
	x->s_off = gensym("off");
	x->s_on = gensym("on");
	x->s_barline = gensym("barline");
	x->s_boxes = gensym("boxes");
	x->s_if = gensym("if");
	x->s_singleslotfortiednotes = gensym("singleslotfortiednotes");
    x->s_copywhensplit = gensym("copywhensplit");
    x->s_follownotehead = gensym("follownotehead");
	x->s_default = gensym("default");
	x->s_quantize = gensym("quantize");
	x->s_zrange = gensym("zrange");
	x->s_zslope = gensym("zslope");

	x->s_display = gensym("display");
	x->s_lowpass = gensym("lowpass");
	x->s_highpass = gensym("highpass");
	x->s_bandpass = gensym("bandpass");
	x->s_bandstop = gensym("bandstop");
	x->s_peaknotch = gensym("peaknotch");
	x->s_lowshelf = gensym("lowshelf");
	x->s_highshelf = gensym("highshelf");
	x->s_resonant = gensym("resonant");
	x->s_allpass = gensym("allpass");
	
	x->s_point = gensym("point");

	x->s_sel = gensym("sel");
	x->s_unsel = gensym("unsel");
    x->s_subsel = gensym("subsel");
	x->s_selmeasures = gensym("selmeasures");
	x->s_unselmeasures = gensym("unselmeasures");

	x->s_leveltype = gensym("leveltype");
	x->s_tupletunit = gensym("tupletunit");
	x->s_tupletdur = gensym("tupletdur");
	x->s_tupletpregressedratio = gensym("tupletpregressedratio");
	x->s_tupletinfo = gensym("tupletinfo");

	x->s_no_name = gensym("no name");
	x->s_empty_symbol = gensym("");
	x->s_setinterleaved = gensym("setinterleaved");
	x->s_transpose = gensym("transpose");
	x->s_complement = gensym("complement");
	x->s_invert = gensym("invert");
	x->s_inf = gensym("inf");
	x->s_error = gensym("error");
	x->s_endeditbox = gensym("endeditbox");
	x->s_frame = gensym("frame");
	x->s_begin_preset = gensym("begin_preset");
	x->s_restore_preset = gensym("restore_preset");
	x->s_end_preset = gensym("end_preset");
	x->s_discardgrace = gensym("discardgrace");
	x->s_discardgracerests = gensym("discardgracerests");
	x->s_trytoavoidgrace = gensym("trytoavoidgrace");
	x->s_gracedeletethresh = gensym("gracedeletethresh");
	x->s_1_16 = gensym("1/16");
	x->s_1_12 = gensym("1/12");
	x->s_vzoom = gensym("vzoom");
	x->s_gridperiodms = gensym("gridperiodms");
	x->s_numgridsubdivisions = gensym("numgridsubdivisions");
	x->s_Courier = gensym("Courier");
	x->s_questionmark = gensym("?");
	x->s_lock = gensym("lock");
	x->s_mute = gensym("mute");
	x->s_solo = gensym("solo");
	x->s_background = gensym("background");
	x->s_popup = gensym("popup");
	x->s_linkto = gensym("linkto");
	x->s_rightclick = gensym("rightclick");
	x->s_lyrics = gensym("lyrics");
	x->s_notecolor = gensym("notecolor");
    x->s_dlcolor = gensym("dlcolor");
	x->s_noteheadadjust = gensym("noteheadadjust");
	x->s_noteheadfont = gensym("noteheadfont");
	x->s_noteheadchar = gensym("noteheadchar");
	x->s_notesize = gensym("notesize");
	x->s_grace = gensym("grace");
	x->s_tempo = gensym("tempo");
    x->s_quartertempo = gensym("quartertempo");
	x->s_figure = gensym("figure");
	x->s_interp = gensym("interp");
	x->s_lockrhythmictree = gensym("lockrhythmictree");
	x->s_clef = gensym("clef");
	x->s_mode = gensym("mode");
	x->s_accpattern = gensym("accpattern");
	x->s_midichannel = gensym("midichannel");
	x->s_attach = gensym("attach");
	x->s_voice = gensym("voice");
	x->s_pim = gensym("pim");
	x->s_stafflines = gensym("stafflines");
	x->s_timesig = gensym("timesig");
	x->s_role = gensym("role");
	x->s_loop = gensym("loop");
    x->s_flags = gensym("flags");
    x->s_path = gensym("path");
	
	x->s_prev = gensym("prev");
	x->s_next = gensym("next");
	
	x->s_notes = gensym("notes");
	x->s_breakpoint = gensym("breakpoint");
	x->s_chords = gensym("chords");
    x->s_rests = gensym("rests");
	x->s_region = gensym("region");
	x->s_scrollbar = gensym("scrollbar");
	x->s_scrollbars = gensym("scrollbars");
	x->s_voices = gensym("voices");
	x->s_tempi = gensym("tempi");
	x->s_slot = gensym("slot");
	x->s_articulation = gensym("articulation");
	x->s_slur = gensym("slur");
	x->s_slurs = gensym("slurs");
	x->s_zoom = gensym("zoom");
	x->s_barlines = gensym("barlines");
	x->s_timesignature =  gensym("timesignature");
	x->s_timesignatures =  gensym("timesignatures");
	x->s_inspector =  gensym("inspector");
	x->s_tails =  gensym("tails");
	x->s_selection =  gensym("selection");
	x->s_group = gensym("group");
	x->s_pitch = gensym("pitch");
	x->s_rhythmictree = gensym("rhythmictree");
	x->s_dilationrectangle = gensym("dilationrectangle");
	x->s_popupmenu = gensym("popupmenu");
	
	
	x->s_create =  gensym("create");
	x->s_delete =  gensym("delete");
	x->s_modify =  gensym("modify");
	x->s_position =  gensym("position");
	x->s_value =  gensym("value");
	

	x->s_names = gensym("names");
	x->s_usecustomnumber = gensym("usecustomnumber");
	x->s_usecustomboxes = gensym("usecustomboxes");
	x->s_number = gensym("number");
	x->s_lockwidth = gensym("lockwidth");
	x->s_separate = gensym("separate");
	x->s_active = gensym("active");
	x->s_openslot = gensym("openslot");
	x->s_sample = gensym("sample");

	x->s_left = gensym("left");
	x->s_right = gensym("right");
	x->s_nearest = gensym("nearest");
	x->s_header = gensym("header");
	x->s_height = gensym("height");
	x->s_root = gensym("root");
	x->s_bachcursors = gensym("bachcursors");
    x->s_getdomains = gensym("getdomains");
    x->s_setdomains = gensym("setdomains");
    x->s_reject = gensym("reject");

	x->s_float64_marker = gensym(LLLL_FLOAT64_MARKER);
	x->s_float64_marker_05 = gensym(LLLL_FLOAT64_MARKER_05);
	x->s_float64_marker_corrupt = gensym(LLLL_FLOAT64_MARKER_CORRUPT);
	
	return x;
}