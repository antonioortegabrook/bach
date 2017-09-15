/**
	@file
	roll.c
	
	@name 
	bach.roll
	
	@realname 
	bach.roll

	@type
	object
	
	@module
	bach

	@author
	bachproject
	
	@digest 
	Display and edit a score in proportional notation
	
	@description
	Displays a score in proportional notation, and provides the interface to interact with it.
	 
	@discussion
	
	@category
	bach, bach objects, bach notation, bach interface, U/I

 	@keywords
	display, edit, proportional, score, note, chord, voice, slot, marker,
	write, open, MIDI, import, export, microtone, 
	duration, cent, midicent, velocity, onset, extra, pitch breakpoint, glissando, 
	pitch, select, enharmonicity, graphic
	
	@palette
	YES
	
	@palette category
	bach, bach objects, bach notation, bach interface

	@seealso
	bach.score, bach.quantize, bach.slot, bach.score2roll, bach.playkeys, bach.slot2line, bach.slot2curve, 
	bach.slot2filtercoeff, bach.ezmidiplay, bach.sliceheader, bach.mono, bach.transcribe, Hello World, Through The Looking Glass, 
	Slot Machines, Real Time Stories, The World Outside, Lambda Scores, Graphical Synchronization, Filter Filter Little Star, Edit Path
	
	@owner
	Daniele Ghisi
*/

#include "llll_modifiers.h"
#include "roll_files.h" //< roll.h is included in here
#include "notation_attrs.h"
#include "notation_goto.h"
#include "ext.h"
#include "ext_obex.h"
#include "jpatcher_api.h"
#include "jgraphics.h"
#include "ext_globalsymbol.h"
#include "ext_systhread.h"
#include "ext_critical.h"
#include <stdio.h>
#include <locale.h>
#include <time.h> 

#ifdef MAC_VERSION
//#define AA_PLAY
#endif

DEFINE_LLLL_ATTR_DEFAULT_GETTER(t_notation_obj, loop_region_as_llll, roll_getattr_loop)

// global class pointer variable
t_class	*s_roll_class = 0;

// verbose?
char verbose = FALSE;

// functions
void roll_inletinfo(t_roll *x, void *b, long a, char *t);
void roll_assist(t_roll *x, void *b, long m, long a, char *s);
void roll_free(t_roll *x);
t_roll* roll_new(t_symbol *s, long argc, t_atom *argv);
void roll_paint(t_roll *x, t_object *view);
t_max_err roll_notify(t_roll *x, t_symbol *s, t_symbol *msg, void *sender, void *data);

void roll_anything(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_int(t_roll *x, t_atom_long num);
void roll_float(t_roll *x, double num);
void roll_bang(t_roll *x);
void roll_clock(t_roll *x, t_symbol *s);


// mute/lock/solo
void roll_mute(t_roll *x);
void roll_unmute(t_roll *x);
void roll_solo(t_roll *x);
void roll_unsolo(t_roll *x);
void roll_lock(t_roll *x);
void roll_unlock(t_roll *x);

void roll_getmaxID(t_roll *x);

void roll_undo(t_roll *x);
void roll_redo(t_roll *x);
void roll_inhibit_undo(t_roll *x, long val);
void roll_prune_last_undo_step(t_roll *x);

void roll_resetslotinfo(t_roll *x);
void roll_resetarticulationinfo(t_roll *x);
void roll_resetnoteheadinfo(t_roll *x);
void roll_distribute(t_roll *x);

void roll_paste_clipboard(t_roll *x, char keep_original_onsets, double force_onset, char keep_original_voice, long force_first_voice, char snap, char also_clear_selection);
void roll_copy_selection(t_roll *x, char cut);

void roll_copy(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_cut(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_paste(t_roll *x, t_symbol *s, long argc, t_atom *argv);


// interface functions
void roll_addchord_from_values(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_subroll(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_merge(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_snap_pitch_to_grid(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_send_current_chord(t_roll *x);

void roll_dump(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void select_all(t_roll *x);

void roll_openslotwin(t_roll *x, t_symbol *s, long argc, t_atom *argv);

// interface functions
void roll_fixvzoom(t_roll *x);
void roll_getdomain(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getdomainpixels(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getpixelpos(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_pixeltotime(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_timetopixel(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_dumpnotepixelpos(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_dumpvoicepixelpos(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_gettimeatpixel(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getvzoom(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getzoom(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getlength(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_domain(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_setdomain(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_clearselection(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getnumvoices(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getnumchords(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getnumnotes(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void send_domain(t_roll *x, long outlet, t_symbol *label);
void roll_resetgraphic(t_roll *x);
void roll_explodechords(t_roll *x);
void roll_exit_linear_edit(t_roll *x);
void roll_linear_edit_move_onset(t_roll *x, long number, char faster, char to_chord_tail_if_chord_is_edited);
void roll_linear_edit_snap_to_chord(t_roll *x);
void roll_linear_edit_snap_cursor_to_grid(t_roll *x);



void roll_delete_selection(t_roll *x, char ripple, t_llll *slots_to_transfer_to_next_note_in_chord_1based = NULL, char transfer_slots_even_if_empty = false);
void roll_delete_selection_and_transfer_default_slots(t_roll *x, char ripple);


// modifying selection
void roll_sel_delete(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_clear_selection(t_roll *x);
void roll_sel_change_onset(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_change_velocity(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_change_cents(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_change_pitch(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_change_voice(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_change_duration(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_change_tail(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_change_ioi(t_roll *x, t_symbol *s, long argc, t_atom *argv); // undocumented, troublesome
void roll_sel_add_breakpoint(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_erase_breakpoints(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_add_slot(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_erase_slot(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_change_slot_value(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_dumpselection(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_sendcommand(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_sel_snap_pitch_to_grid(t_roll *x);
void roll_sel_snap_onset_to_grid(t_roll *x);
void roll_sel_snap_tail_to_grid(t_roll *x);
void roll_sel_resetgraphic(t_roll *x);
void roll_legato(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_glissando(t_roll *x, t_symbol *s, long argc, t_atom *argv);
char roll_sel_dilate_ms(t_roll *x, double ms_factor, double fixed_ms_point);
char roll_sel_dilate_mc(t_roll *x, double mc_factor, double fixed_mc_point);

// wrapped functions in roll_legato and roll_sel_change_tail 
char legato(t_roll *x, long mode);
char glissando(t_roll *x, long mode);
char roll_do_sel_change_tail(t_roll *x, t_symbol *s, long argc, t_atom *argv);

void roll_name(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_nameappend(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_slottoname(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_nametoslot(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_clearnames(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_select(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_role(t_roll *x, t_symbol *s, long argc, t_atom *argv);

void roll_group(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_ungroup(t_roll *x, t_symbol *s, long argc, t_atom *argv);



void roll_inscreen(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_inscreenpos(t_roll *x, t_symbol *s, long argc, t_atom *argv);

void roll_lambda(t_roll *x, t_symbol *s, long argc, t_atom *argv);

// markers
void roll_addmarker(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_deletemarker(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_clearmarkers(t_roll *x);
void roll_markername(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getmarkers(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_getmarker(t_roll *x, t_symbol *s, long argc, t_atom *argv);

// modifying cursor
void roll_setcursor(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_hidecursor(t_roll *x);
void roll_showcursor(t_roll *x);
void roll_getcursor(t_roll *x);
void roll_getloop(t_roll *x);


// inscreen
char force_inscreen_ms(t_roll *x, double inscreen_ms, char send_domain_if_changed);
char force_inscreenpos_ms(t_roll *x, double position, double inscreen_ms, char send_domain, char also_check_scheduling, char also_move_mousedown_pt);
char force_inscreen_ms_to_boundary(t_roll *x, double inscreen_ms, char clip_to_length, char send_domain_if_changed, char also_check_scheduling, char also_move_mousedown_pt);
char force_inscreen_ms_rolling(t_roll *x, double inscreen_ms, char clip_to_length, char send_domain_if_changed, char also_check_scheduling, char also_move_mousedown_pt);

void roll_declare_bach_attributes(t_roll *x);

void roll_delete_voice(t_roll *x, t_rollvoice *voice);
void roll_delete_voiceensemble(t_roll *x, t_voice *any_voice_in_voice_ensemble);
void roll_move_and_reinitialize_last_voice(t_roll *x, t_rollvoice *after_this_voice, t_symbol *key, long clef, t_llll *voicename, long midichannel, long idx_of_the_stafflist_element_in_llll);
void roll_swap_voices(t_roll *x, t_rollvoice *v1, t_rollvoice *v2);
void roll_swap_voiceensembles(t_roll *x, t_rollvoice *v1, t_rollvoice *v2);

// setters
t_max_err roll_setattr_showstems(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_nonantialiasedstaff(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_clefs(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_keys(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_tonedivision(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_accidentalsgraphic(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_accidentalspreferences(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_inset(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_zoom(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_vzoom(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_linkvzoomtoheight(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_view(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_noteheads_font(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_accidentals_font(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_articulations_font(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_hidevoices(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_voicespacing(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_showlyrics(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_enharmonictable(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_loop(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_minlength(t_roll *x, t_object *attr, long ac, t_atom *av);
t_max_err roll_setattr_customspacing(t_roll *x, t_object *attr, long ac, t_atom *av);

// mouse functions
void roll_mouseenter(t_roll *x, t_object *patcherview, t_pt pt, long modifiers);
void roll_mouseleave(t_roll *x, t_object *patcherview, t_pt pt, long modifiers);
void roll_mousemove(t_roll *x, t_object *patcherview, t_pt pt, long modifiers);
void roll_mousedrag(t_roll *x, t_object *patcherview, t_pt pt, long modifiers);
void roll_mousedown(t_roll *x, t_object *patcherview, t_pt pt, long modifiers);
void roll_mouseup(t_roll *x, t_object *patcherview, t_pt pt, long modifiers);
void roll_mousewheel(t_roll *x, t_object *view, t_pt pt, long modifiers, double x_inc, double y_inc);
long roll_oksize(t_roll *x, t_rect *newrect);
t_atom_long roll_acceptsdrag_unlocked(t_roll *x, t_object *drag, t_object *view);
long roll_key(t_roll *x, t_object *patcherview, long keycode, long modifiers, long textcharacter);
long roll_keyfilter(t_roll *x, t_object *patcherview, long *keycode, long *modifiers, long *textcharacter);
void roll_mousedoubleclick(t_roll *x, t_object *patcherview, t_pt pt, long modifiers);
void roll_focusgained(t_roll *x, t_object *patcherview);
void roll_focuslost(t_roll *x, t_object *patcherview);
void roll_enter(t_roll *x);
void roll_edclose(t_roll *x, char **ht, long size);
void roll_okclose(t_roll *x, char *s, short *result);
void roll_jsave(t_roll *x, t_dictionary *d);
void roll_preset(t_roll *x);
void roll_begin_preset(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_restore_preset(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_end_preset(t_roll *x);
void roll_play(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_do_play(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_playselection(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_stop(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_do_stop(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_pause(t_roll *x);
void roll_play_offline(t_roll *x, t_symbol *s, long argc, t_atom *argv);
void roll_task(t_roll *x);

void roll_adjustadditionalstartpad(t_roll *x);



/**	Snap the onset of all the selected chords to the time grid (if any). Currently it only works for [bach.roll].
	@ingroup		notation_actions
	@param	r_ob	The notation object
	@see			snap_onset_to_grid_for_chord()
 */ 
char snap_onset_to_grid_for_selection(t_roll *x);



// auxiliary functions
t_note* ID_to_note(t_roll *x, long ID); //almost unused
t_chord* ID_to_chord(t_roll *x, long ID); //almost unused
void verbose_print(t_roll *x);
t_rollvoice* nth_rollvoice(t_roll *x, long n);	// 0-based!!!!!
t_chord* nth_chord_of_rollvoice(t_rollvoice *voice, long n);	/// 1-based!!!!
t_chord* nth_marker(t_rollvoice *voice, long n);	/// 1-based!!!!

// llll communication functions
t_llll* get_rollvoice_values_as_llll(t_roll *x, t_rollvoice *voice, e_data_considering_types for_what);
t_llll* get_onsets_values_as_llll(t_roll *x);
t_llll* get_cents_values_as_llll(t_roll *x);
t_llll* get_durations_values_as_llll(t_roll *x);
t_llll* get_velocities_values_as_llll(t_roll *x);
t_llll* get_extras_values_as_llll(t_roll *x);
t_llll* get_pixel_values_as_llll(t_roll *x);
t_llll* get_voice_onsets_values_as_llll(t_rollvoice *voice);
t_llll* get_voice_cents_values_as_llll(t_roll *x, t_rollvoice *voice);
t_llll* get_voice_durations_values_as_llll(t_rollvoice *voice);
t_llll* get_voice_velocities_values_as_llll(t_rollvoice *voice);
t_llll* get_voice_pixel_values_as_llll(t_roll *x, t_rollvoice *voice);
t_llll *get_selection_gathered_syntax(t_roll *x);

//t_llll* get_voice_extras_values_as_llll(t_roll *x, t_rollvoice *voice);
void send_all_values_as_llll(t_roll *x, e_header_elems send_what);
void send_subroll_values_as_llll(t_roll *x, t_llll* whichvoices, double start_ms, double end_ms, t_llll *what_to_dump, char subroll_type);
t_llll* get_subroll_values_as_llll(t_roll *x, t_llll* whichvoices, double start_ms, double end_ms, t_llll *what_to_dump, char subroll_type);
t_llll* get_subvoice_values_as_llll(t_roll *x, t_rollvoice *voice, double start_ms, double end_ms, char subroll_type);
void send_roll_values_as_llll(t_roll *x, e_header_elems send_what);
void send_onsets_values_as_llll(t_roll *x);
void send_cents_values_as_llll(t_roll *x);
void send_durations_values_as_llll(t_roll *x);
void send_velocities_values_as_llll(t_roll *x);
void send_extras_values_as_llll(t_roll *x);
void set_voice_onsets_values_from_llll(t_roll *x, t_llll* onsets, t_rollvoice *voice, char also_check_chords_order);
void set_voice_cents_values_from_llll(t_roll *x, t_llll* cents, t_rollvoice *voice, char force_append_notes);
void set_voice_durations_values_from_llll(t_roll *x, t_llll* durations, t_rollvoice *voice);
void set_voice_velocities_values_from_llll(t_roll *x, t_llll* velocities, t_rollvoice *voice);
void set_voice_graphic_values_from_llll(t_roll *x, t_llll* graphics, t_rollvoice *voice);
void set_voice_breakpoints_values_from_llll(t_roll *x, t_llll* breakpoints, t_rollvoice *voice);
void set_voice_slots_values_from_llll(t_roll *x, t_llll* slots, t_rollvoice *voice);
void set_onsets_values_from_llll(t_roll *x, t_llll* onsets, char also_check_chords_order, long num_introduced_voices);
void set_cents_values_from_llll(t_roll *x, t_llll* cents, char force_append_notes, long num_introduced_voices);
void set_durations_values_from_llll(t_roll *x, t_llll* durations, long num_introduced_voices);
void set_velocities_values_from_llll(t_roll *x, t_llll* velocities, long num_introduced_voices);
void set_extras_values_from_llll(t_roll *x, t_llll* extras);
void set_graphic_values_from_llll(t_roll *x, t_llll* graphics);
void set_breakpoints_values_from_llll(t_roll *x, t_llll* breakpoints);
void set_slots_values_from_llll(t_roll *x, t_llll* slots);
void set_roll_from_llll(t_roll *x, t_llll* inputlist, char also_lock_general_mutex);
t_chord *addchord_from_llll(t_roll *x, t_llll* chord, t_rollvoice *voice, char also_lock_general_mutex, char also_recompute_total_length);
void gluechord_from_llll(t_roll *x, t_llll* chord, t_rollvoice *voice, double threshold_ms, double threshold_cents, double smooth_ms);

t_llll* get_roll_values_as_llll_for_pwgl(t_roll *x);

void changed_bang(t_roll *x, int change_type);

long get_global_num_notes_voice(t_rollvoice *voice);
void recalculate_all_chord_parameters(t_roll *x);
char merge(t_roll *x, double threshold_ms, double threshold_cents, char gathering_policy_ms, char gathering_policy_cents, char only_selected, char markers_also);
char check_chords_order_for_voice(t_roll *x, t_rollvoice *voice);
void check_all_chords_order(t_roll *x);
void check_all_chords_and_notes_order(t_roll *x);
void check_all_chords_order_and_correct_scheduling_fn(t_bach_inspector_manager *man, void *obj, t_bach_attribute *attr);
void clear_voice(t_roll *x, t_rollvoice *voice);
void clear_roll_body(t_roll *x, long voicenum);
void evaluate_selection(t_roll *x, long modifiers, char alsosortselectionbyonset, t_llll *forced_routers = NULL);
void selection_send_command(t_roll *x, long modifiers, long command_number, char alsosortselectionbyonset);
double get_selection_leftmost_onset(t_roll *x);
double get_selection_rightmost_onset(t_roll *x);
long get_selection_topmost_voice(t_roll *x);

char change_pitch_for_selection(t_roll *x, double delta, char mode, char allow_voice_change, char snap_pitch_to_grid);
t_chord *shift_note_allow_voice_change(t_roll *x, t_note *note, double delta, char mode, char *old_chord_deleted, char allow_voice_change);
void snap_pitch_to_grid_voice(t_roll *x, t_rollvoice *voice);


t_chord* addchord_from_values(t_roll *x, long voicenumber, long num_notes, double onset, long unused, long argc, double *argv, long* num_bpt, double *bpt, long slot_list_length, t_atom *slots, char force_append, unsigned long force_ID, unsigned long *force_note_IDs, char also_recompute_total_length);
t_chord* addchord_from_notes(t_roll *x, long voicenumber, double onset, long unused, long num_notes, t_note *firstnote, t_note *lastnote, char force_append, unsigned long force_ID);
void change_note_duration(t_roll *x, t_note *note, double delta_ms);
char is_note_in_selected_region(t_roll *x, t_chord *chord, t_note *note);
char is_chord_in_selected_region(t_roll *x, t_chord *chord);
char equally_respace_selection_onsets(t_roll *x);
char change_selection_duration(t_roll *x, double delta_ms); 
char change_selection_onset(t_roll *x, double *delta_ms);
//char change_selection_tail(t_roll *x, double delta_ms);  
char align_selection_onsets(t_roll *x);
char move_selection_breakpoint(t_roll *x, double delta_x_pos, double delta_y_pos, char tail_only);
//char delete_selection(t_roll *x, char only_editable_items);
void set_everything_unplayed(t_roll *x);
void process_chord_parameters_calculation_NOW(t_roll *x);
char change_note_voice_from_lexpr_or_llll(t_roll *x, t_note *note, t_lexpr *lexpr, t_llll *new_voice, char also_select);
char change_chord_voice_from_lexpr_or_llll(t_roll *x, t_chord *chord, t_lexpr *lexpr, t_llll *new_voice, char also_select);

void create_whole_roll_undo_tick(t_roll *x);
void roll_clear_all(t_roll *x);


// Nonstandard accidental display (e.g. naturalizations, etc. see #e_show_accidentals_preferences)
void update_all_accidentals_if_needed(t_roll *x);
void update_all_accidentals_for_voice_if_needed(t_roll *x, t_rollvoice *voice);
void update_all_accidentals_for_chord_if_needed(t_roll *x, t_chord *chord);


// bach inspector
void force_notation_item_inscreen(t_roll *x, t_notation_item *it, void *dummy);

// selections
void preselect_elements_in_region_for_mouse_selection(t_roll *x, double ms1, double ms2, double mc1, double mc2, long v1, long v2);

// quantization
void roll_quantize(t_roll *x, t_symbol *s, long argc, t_atom *argv);

DEFINE_NOTATIONOBJ_SYMPTR_GETTER(clefs_as_symlist, num_voices)
DEFINE_NOTATIONOBJ_SYMPTR_GETTER(keys_as_symlist, num_voices)
DEFINE_NOTATIONOBJ_SYMPTR_GETTER(full_acc_repr, num_voices)
DEFINE_NOTATIONOBJ_CHARPTR_GETTER(hidevoices_as_charlist, num_voices)
DEFINE_NOTATIONOBJ_DBLPTR_GETTER(voiceuspacing_as_floatlist, num_voices_plus_one)

// clipboard
t_clipboard clipboard = {k_NONE, k_NOTATION_OBJECT_ROLL, NULL, NULL, 0.};


t_max_err roll_notify(t_roll *x, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
    notationobj_handle_attr_modified_notify ((t_notation_obj *)x, s, msg, sender, data);
    return jbox_notify((t_jbox *)x, s, msg, sender, data);
}

t_llll* get_roll_values_as_llll_for_pwgl(t_roll *x){
// get all the information concerning the score in order to perform the xml import/export
	t_llll* out_llll = llll_get();
	t_rollvoice *voice;
	t_chord *temp_chord; 
	t_note *temp_note;
	lock_general_mutex((t_notation_obj *)x);	
	
	x->r_ob.private_count = 1; // needed for slots: for PWGL each slot is a different instance of an objects, we'll keep the count with this

	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) { // append chord lllls

		t_llll* voice_llll = llll_get();

		temp_chord = voice->firstchord;
			
		while (temp_chord) { // append chord lllls
			t_llll* chord_llll = llll_get();
			t_llll* notes_llll = llll_get();
			double min_chord_duration_ms = 0.;

			llll_appenddouble(chord_llll, temp_chord->onset / 1000., 0, WHITENULL_llll);
			llll_appendsym(chord_llll, gensym(":notes"), 0, WHITENULL_llll);

			for (temp_note = temp_chord->firstnote; temp_note; temp_note = temp_note->next)
				if (min_chord_duration_ms == 0. || temp_note->duration < min_chord_duration_ms)
					min_chord_duration_ms = temp_note->duration;

			temp_note = temp_chord->firstnote;
			while (temp_note) {
				t_llll* this_note_llll = llll_get();
				double mc_as_double = rat2double(note_get_screen_midicents_with_accidental(temp_note))/100.;

				if (((double)mc_as_double) == round(mc_as_double))
					llll_appendlong(this_note_llll, round(mc_as_double), 0, WHITENULL_llll);
				else
					llll_appenddouble(this_note_llll, mc_as_double, 0, WHITENULL_llll);
				if (are_note_breakpoints_nontrivial((t_notation_obj *) x,temp_note)) 
					append_note_breakpoints_formatted_for_pwgl((t_notation_obj *) x, this_note_llll, temp_note);
				if (notation_item_has_slot_content((t_notation_obj *) x, (t_notation_item *)temp_note))
					append_note_slot_formatted_for_pwgl((t_notation_obj *) x, this_note_llll, temp_note);
				llll_appendsym(this_note_llll, gensym(":offset-dur"), 0, WHITENULL_llll);
				llll_appenddouble(this_note_llll, (temp_note->duration - min_chord_duration_ms) / 1000., 0, WHITENULL_llll);
				llll_appendllll(notes_llll, this_note_llll, 0, WHITENULL_llll);	
				temp_note = temp_note->next;
			}
			llll_appendllll(chord_llll, notes_llll, 0, WHITENULL_llll);	
			
			llll_appendsym(chord_llll, gensym(":duration"), 0, WHITENULL_llll);
			llll_appenddouble(chord_llll, min_chord_duration_ms / 1000., 0, WHITENULL_llll);

			llll_appendllll(voice_llll, chord_llll, 0, WHITENULL_llll);
			temp_chord = temp_chord->next;
		}
		
		llll_wrap_once(&voice_llll);
		llll_appendllll(out_llll, voice_llll, 0, WHITENULL_llll);	
		voice = voice->next;
	}
	
	llll_wrap_once(&out_llll);
	unlock_general_mutex((t_notation_obj *)x);	

	return out_llll;
}


void remove_just_added_from_separate_parameters_flag(t_roll *x)
{
	t_rollvoice *voice;
	t_chord *chord;
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next)
		for (chord = voice->firstchord; chord; chord = chord->next)
			chord->just_added_from_separate_parameters = false;
}

void roll_bang(t_roll *x)
{
// reconstruct roll!

	long i;
	t_llll *inlist[6];
	t_llll *onsets, *cents, *durations, *velocities, *extras;
	char more_than_one_extra_defined = false;
	
	for (i = 0; i < 6; i++) 
		inlist[i] = llllobj_get_store_contents((t_object *) x, LLLL_OBJ_UI, i, 1);

	if (inlist[5] && inlist[5]->l_head && hatom_gettype(&inlist[5]->l_head->l_hatom) == H_LLLL)
		more_than_one_extra_defined = true;
		
	create_whole_roll_undo_tick(x); 

	lock_general_mutex((t_notation_obj *)x);	

	if (x->must_append_chords) {	// mode "addchords": we add some nils in each voice for each skip of existing chords!
		long j;
		for (i = 1; i <= 5; i++) {
			if (inlist[i]) {
				t_llllelem *elem; 
				t_rollvoice *voice = x->firstvoice;
				t_llllelem *startelem = (i == 5 && !more_than_one_extra_defined && inlist[i]->l_head) ? inlist[i]->l_head->l_next : inlist[i]->l_head;
				
				for (elem = startelem; elem && voice; elem = elem->l_next) {
					if (hatom_gettype(&elem->l_hatom) == H_LLLL) {
						if (i == 5 && more_than_one_extra_defined) { // specific case when more than 1 extra is defined
							t_llll *this_extra = hatom_getllll(&elem->l_hatom);
							if (this_extra && this_extra->l_head && hatom_gettype(&this_extra->l_head->l_hatom) == H_SYM && this_extra->l_head->l_next) {
								t_llllelem *elem2;
								voice = x->firstvoice;
								for (elem2 = this_extra->l_head->l_next; elem2 && voice; elem2 = elem2->l_next) {
									if (hatom_gettype(&elem2->l_hatom) == H_LLLL) {
										for (j = 0; j < voice->num_chords; j++)
											llll_prependllll(hatom_getllll(&elem2->l_hatom), llll_get(), 0, WHITENULL_llll);
										voice = voice->next;
									}
								}
							}
						} else {
							for (j = 0; j < voice->num_chords; j++)
								llll_prependllll(hatom_getllll(&elem->l_hatom), llll_get(), 0, WHITENULL_llll);
							voice = voice->next;
						}
					}
				}
			}
		}
	} else {
		if (x->r_ob.autoclear)
			roll_clear_all(x);
	}
	
	onsets = inlist[1];
	cents = inlist[2];
	durations = inlist[3];
	velocities = inlist[4];
	extras = inlist[5];
	
	// detecting number of voices and auto-sizing if needed
    long num_introduced_voices = -1; // = don't change
    num_introduced_voices = MAX(num_introduced_voices, (long)onsets->l_size);
    num_introduced_voices = MAX(num_introduced_voices, (long)cents->l_size);
    num_introduced_voices = MAX(num_introduced_voices, (long)durations->l_size);
    num_introduced_voices = MAX(num_introduced_voices, (long)velocities->l_size);
    if (extras && extras->l_head) {
        if (hatom_gettype(&extras->l_head->l_hatom) == H_SYM) // just 1 extra defined
            num_introduced_voices = MAX(num_introduced_voices, (long)extras->l_size - 1);
        else {
            t_llllelem *elem;
            for (elem = extras->l_head; elem; elem = elem->l_next) {
                if (hatom_gettype(&elem->l_hatom) == H_LLLL) {
                    t_llll *ll = hatom_getllll(&elem->l_hatom);
                    if (ll->l_head && hatom_gettype(&ll->l_head->l_hatom) == H_SYM)
                        num_introduced_voices = MAX(num_introduced_voices, (long)ll->l_size - 1);
                }
            }
        }
    }
    
    // we change the number of voices if there was no "addchords" message and the number of voice inserted is different from the existing one,
    // or if there was "addchords" message, and the number of voices inserted is greater than the existing ones.
    if (x->r_ob.autosize) {
		if (num_introduced_voices > 0 &&
			((!x->must_append_chords && x->r_ob.num_voices != num_introduced_voices) ||
			 (x->must_append_chords && x->r_ob.num_voices < num_introduced_voices))) {
				set_numvoices((t_notation_obj *)x, num_introduced_voices);
		}
	}
	
	
	// reconstruct roll from separate parameters
	if (onsets && (onsets->l_size > 0)) {
		set_onsets_values_from_llll(x, onsets, false, num_introduced_voices);
		llllobj_store_llll((t_object *) x, LLLL_OBJ_UI, llll_get(), 1);
	}
	if (cents && (cents->l_size > 0)) {
		set_cents_values_from_llll(x, cents, true, num_introduced_voices);
		llllobj_store_llll((t_object *) x, LLLL_OBJ_UI, llll_get(), 2);
	}
	if (durations && (durations->l_size > 0)) {
		set_durations_values_from_llll(x, durations, num_introduced_voices);
		llllobj_store_llll((t_object *) x, LLLL_OBJ_UI, llll_get(), 3);
	}
	if (velocities && (velocities->l_size > 0)) {
		set_velocities_values_from_llll(x, velocities, num_introduced_voices);
		llllobj_store_llll((t_object *) x, LLLL_OBJ_UI, llll_get(), 4);
	}
	if (extras && (extras->l_size > 0)) {
		set_extras_values_from_llll(x, extras);
		llllobj_store_llll((t_object *) x, LLLL_OBJ_UI, llll_get(), 5);
	}
	
	remove_just_added_from_separate_parameters_flag(x);
	check_all_chords_and_notes_order(x);
	process_chord_parameters_calculation_NOW(x);
	
	unlock_general_mutex((t_notation_obj *)x);	

	x->must_append_chords = false;
	x->must_preselect_appended_chords = false;
	x->must_apply_delta_onset = 0.;
	
	for (i = 0; i < 6; i++)
		llll_free(inlist[i]);

	handle_rebuild_done((t_notation_obj *) x);

	recalculate_all_chord_parameters(x);
	x->r_ob.hscrollbar_pos = 0.; // needed here, to initialize r_ob->screen_ux_start
	update_hscrollbar((t_notation_obj *)x, 1);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);

	handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_ROLL);
}


// OBSOLETE FUNCTION!!!! NOW IT IS NEVER CALLED!!!!
// change_type: 0 = calculateUNDOSTEP; 1 = BANG+calculateUNDOSTEPonly; 2 = BANGonly
void changed_bang(t_roll *x, int change_type){
	
	if (change_type & k_CHANGED_REDRAW_STATIC_LAYER){
		jbox_invalidate_layer((t_object *)x, NULL, gensym("static_layer1"));
		jbox_invalidate_layer((t_object *)x, NULL, gensym("static_layer2"));
	}
	
	if (change_type == k_CHANGED_REDRAW_STATIC_LAYER) {
        notationobj_redraw((t_notation_obj *) x);
		return; // nothing more to do!
	}
	
	
	if (!USE_NEW_UNDO_SYSTEM) {
		if (!x->r_ob.j_isdragging && (change_type & k_CHANGED_CREATE_UNDO_STEP)) {
			
			if (x->r_ob.save_data_with_patcher && !x->r_ob.j_box.l_dictll) // set dirty flag
				object_attr_setchar(x->r_ob.patcher_parent, gensym("dirty"), 1);
			
			if (x->r_ob.allow_undo){
				// update undo and redo lists
				
				t_llll *kill_me_redo[CONST_MAX_UNDO_STEPS];
				t_llll *kill_me_undo; 
				int i;
				
				lock_general_mutex((t_notation_obj *)x);	
				for (i = 0; i< CONST_MAX_UNDO_STEPS; i++) { // deleting redolist
					kill_me_redo[i] = x->r_ob.old_redo_llll[i];
					x->r_ob.old_redo_llll[i] = NULL;
				}
				
				// deleting last element of the undo list
				kill_me_undo = x->r_ob.old_undo_llll[CONST_MAX_UNDO_STEPS - 1];
				
				for (i = CONST_MAX_UNDO_STEPS - 1; i > 0; i--) // reassign undo steps
					x->r_ob.old_undo_llll[i] = x->r_ob.old_undo_llll[i-1];
				unlock_general_mutex((t_notation_obj *)x);	
				
				// killing elements
				for (i = 0; i < CONST_MAX_UNDO_STEPS; i++) {
					if (kill_me_redo[i])
						llll_free(kill_me_redo[i]);
				}
				if (kill_me_undo) 
					llll_free(kill_me_undo);
				
				// setting first element of the list
				x->r_ob.old_undo_llll[0] = get_roll_values_as_llll(x, k_CONSIDER_FOR_SAVING, 
																   (e_header_elems) (k_HEADER_BODY | k_HEADER_SLOTINFO | k_HEADER_VOICENAMES | k_HEADER_MARKERS | k_HEADER_GROUPS | k_HEADER_ARTICULATIONINFO | k_HEADER_NOTEHEADINFO | k_HEADER_NUMPARTS), true, false); // we don't undo clefs, key, midichannels and commands changes
				
				// setting new undo time
				x->r_ob.last_undo_time = systime_ms();
			}
		}
	}

	if (change_type & k_CHANGED_CHECK_CORRECT_SCHEDULING)
		check_correct_scheduling((t_notation_obj *)x, true);
	
	// in any case:
    notationobj_redraw((t_notation_obj *) x);

	if (change_type & k_CHANGED_SEND_BANG)
		llllobj_outlet_bang((t_object *) x, LLLL_OBJ_UI, 7);
	
	if (x->r_ob.automessage_ac > 0 && !x->r_ob.itsme && (change_type & k_CHANGED_SEND_AUTOMESSAGE)){
		t_atom result;
		x->r_ob.itsme = true;
		x->r_ob.is_sending_automessage = true;
		object_method_typed(x, NULL, x->r_ob.automessage_ac, x->r_ob.automessage_av, &result);
		x->r_ob.is_sending_automessage = false;
		x->r_ob.itsme = false; 
	}
}



void roll_begin_preset(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	x->r_ob.preset_ac = atom_getlong(argv) ;
	x->r_ob.preset_av = (t_atom *)bach_newptr(x->r_ob.preset_ac * sizeof(t_atom));
	
	// could allocate memory here etc.
//	post("begin_preset %ld values will be arriving",x->r_ob.preset_ac);
}


void roll_restore_preset(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	long start, size;
	
	start = atom_getlong(argv);
	size = atom_getlong(argv + 1);
//	post("restore_preset start %ld size %ld", start,size);
	
	sysmem_copyptr(argv+2, x->r_ob.preset_av+start, size*sizeof(t_atom));
}

void roll_end_preset(t_roll *x)
{
//	post("end_preset received");
	roll_anything(x, NULL, x->r_ob.preset_ac,x->r_ob.preset_av);
	bach_freeptr(x->r_ob.preset_av);
}

void roll_preset(t_roll *x)
{
	t_atom temp[256];
	void *buf;
	long i, index, size;
	char wrote = false, begin = true, done = false;
	t_llll *whole_info = get_roll_values_as_llll(x, k_CONSIDER_FOR_SAVING, k_HEADER_ALL, true, false); // we save all

//    dev_post("preset_AA");
//    llll_post(whole_info);
    
	// 1. we deparse the list
	t_atom *av = NULL, *this_av = NULL;
	long ac = llll_deparse(whole_info, &av, 0, /* LLLL_D_FLOAT64 | */ LLLL_D_QUOTE); //offset 0
	this_av = av;
	
/*    dev_post("preset_BB");
    for (long j = 0; j < ac; j++)
        postatom(av + j);
  */
    
	// 2. We fill the binbuf
	buf = gensym("_preset")->s_thing; 
	if (!buf)	// no preset object
		return;
		
	index = 0;
	while (!done) {
		i = index;
		size = ac - index;
		if (size > 250)
			size = 250;
		else	// you are at the end
			done = true;
		
		sysmem_copyptr(this_av, temp+5, 250*sizeof(t_atom));

		if (size) {
			atom_setobj(temp, x);
			atom_setsym(temp + 1, ob_sym(x));
			if (begin) {
				atom_setsym(temp + 2, gensym("begin_preset"));
				atom_setlong(temp + 3, ac);
				binbuf_insert(buf, NULL, 4, temp);
				begin = false;
			}
			atom_setsym(temp + 2, gensym("restore_preset"));
			atom_setlong(temp + 3, index);
			atom_setlong(temp + 4, size);
			binbuf_insert(buf, NULL, size + 5, temp);
			wrote = true;
		}
		index += size;
		this_av += 250;
	}
	if (wrote) {
		atom_setobj(temp, x);
		atom_setsym(temp + 1, ob_sym(x));
		atom_setsym(temp + 2, gensym("end_preset"));
		binbuf_insert(buf, NULL, 3, temp);
	}
		
	if (av) bach_freeptr(av);
}

/* preset: Zicarelli


	t_atom temp[256];
	// imagine that we have
	reallylongdata
	
	long i, j, index;
	char wrote = false, begin = true;
	index = 0;
	for (; ; ) {
		i = index;
		size = x->s_length - index;
		if (size > 250)
			size = 250;
	}
	
	...
	
myobject_begin_preset allocates a size (how much data?)
myobject_recall_preset contains the data 
myobject_end_preset 

*/

// *******************************************************************************************
// communication with bach.quantize
// *******************************************************************************************

void roll_quantize(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	// we send a message like
	// quantize <cents> <durations> <velocities> <ties> <extras>
	// formatted for quantization
	
	t_llll *out_llll = llll_get();
	
	t_llll *out_cents = llll_get();
	t_llll *out_durations = llll_get();
	t_llll *out_velocities = llll_get();
	t_llll *out_ties = llll_get();
	t_llll *out_graphic = llll_get();
	t_llll *out_breakpoints = llll_get();
	t_llll *out_slots = llll_get();
	t_llll *out_extras = llll_get();

	t_llll *what_to_dump_llll;
	long what_to_dump = k_HEADER_ALL;

	t_chord *chord;
	t_rollvoice *voice;

	llll_appendsym(out_graphic, _llllobj_sym_graphic, 0, WHITENULL_llll);
	llll_appendsym(out_breakpoints, _llllobj_sym_breakpoints, 0, WHITENULL_llll);
	llll_appendsym(out_slots, _llllobj_sym_slots, 0, WHITENULL_llll);

	lock_general_mutex((t_notation_obj *)x);
	for (voice = x->firstvoice;	voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
		t_llll *out_voice_cents = llll_get();
		t_llll *out_voice_durations = llll_get();
		t_llll *out_voice_velocities = llll_get();
		t_llll *out_voice_ties = llll_get();
		t_llll *out_voice_graphic = llll_get();
		t_llll *out_voice_breakpoints = llll_get();
		t_llll *out_voice_slots = llll_get();

		t_llll *active_cents = llll_get();
		t_llll *active_velocities = llll_get();
		t_llll *active_graphic = llll_get();
		t_llll *active_slots = llll_get();
		t_llll *active_until = llll_get(); // ms of end activity
		double curr_onset = 0;

		// is there a pause before the beginning of the first chord ?
		if (voice->firstchord && voice->firstchord->onset > 0) {
			// add a pause
			llll_appendllll(out_voice_cents, llll_get(), 0, WHITENULL_llll); 
			llll_appenddouble(out_voice_durations, -voice->firstchord->onset, 0, WHITENULL_llll); // pause: negative
			llll_appendllll(out_voice_velocities, llll_get(), 0, WHITENULL_llll); 
			llll_appendllll(out_voice_ties, llll_get(), 0, WHITENULL_llll); 
			llll_appendllll(out_voice_graphic, llll_get(), 0, WHITENULL_llll); 
			llll_appendllll(out_voice_breakpoints, llll_get(), 0, WHITENULL_llll); 
			llll_appendllll(out_voice_slots, llll_get(), 0, WHITENULL_llll); 
		}

		for (chord = voice->firstchord; chord; chord = chord->next) {
			t_note *note;
			t_llll *this_event_cents = llll_get();
			t_llll *this_event_velocities = llll_get();
			t_llll *this_event_ties = llll_get();
			t_llll *this_event_graphic = llll_get();
			t_llll *this_event_breakpoints = llll_get();
			t_llll *this_event_slots = llll_get();
			double next_onset;
			t_chord *tmp_chord;

			t_llllelem *active_cents_elem, *active_velocities_elem, *active_until_elem, *active_graphic_elem, *active_slots_elem;
			t_chord *lastchord_of_group = voice->lastchord;
			t_llllelem *temp1, *temp2, *temp3, *temp4, *temp5;

			curr_onset = chord->onset;
			
			// first: we find next event start
			next_onset = chord->onset + chord->firstnote->duration; // just to inizialize properly
			tmp_chord = chord;
			while (tmp_chord->next) {
				if (tmp_chord->next->onset > chord->onset) {
					if (tmp_chord->next->onset < next_onset) 
						next_onset = tmp_chord->next->onset;
					lastchord_of_group = tmp_chord;
					break; // otherwise we gather all the chords having the same onset
				}
				tmp_chord = tmp_chord->next;
			}
			
			for (tmp_chord = chord; tmp_chord; tmp_chord = tmp_chord->next){
				for (note = tmp_chord->firstnote; note; note = note->next){
					if (tmp_chord->onset + note->duration < next_onset)
						next_onset = tmp_chord->onset + note->duration;
				}
				if (tmp_chord == lastchord_of_group)
					break;
			}
			
			for (active_until_elem = active_until->l_head; active_until_elem; active_until_elem = active_until_elem->l_next) {
				double this_onset = hatom_getdouble(&active_until_elem->l_hatom);
				if (this_onset < next_onset)
					next_onset = this_onset;
			}

			// third: we build our event-lists
			active_cents_elem = active_cents->l_head; 
			active_velocities_elem = active_velocities->l_head; 
			active_graphic_elem = active_graphic->l_head; 
			active_slots_elem = active_slots->l_head; 
			active_until_elem = active_until->l_head;
			while (active_cents_elem && active_velocities_elem && active_until_elem && active_graphic_elem && active_slots_elem) {
				double end_of_this_event = hatom_getdouble(&active_until_elem->l_hatom);
				llll_appenddouble(this_event_cents, hatom_getdouble(&active_cents_elem->l_hatom), 0, WHITENULL_llll);
				llll_appendlong(this_event_velocities, hatom_getdouble(&active_velocities_elem->l_hatom), 0, WHITENULL_llll);
				llll_appendhatom_clone(this_event_graphic, &active_graphic_elem->l_hatom, 0, WHITENULL_llll);
				llll_appendllll(this_event_breakpoints, llll_get(), 0, WHITENULL_llll);
				llll_appendhatom_clone(this_event_slots, &active_slots_elem->l_hatom, 0, WHITENULL_llll);
				if (end_of_this_event == next_onset) {
					llll_appendlong(this_event_ties, 0, 0, WHITENULL_llll);
					// the event is no more active!
					temp1 = active_cents_elem->l_next;
					temp2 = active_velocities_elem->l_next;
					temp3 = active_graphic_elem->l_next;
					temp4 = active_until_elem->l_next;
					temp5 = active_slots_elem->l_next;
					llll_destroyelem(active_cents_elem);
					llll_destroyelem(active_velocities_elem);
					llll_destroyelem(active_graphic_elem);
					llll_destroyelem(active_slots_elem);
					llll_destroyelem(active_until_elem);
					active_cents_elem = temp1; 
					active_velocities_elem = temp2; 
					active_graphic_elem = temp3;
					active_until_elem = temp4;
					active_slots_elem = temp5;
				} else {
					llll_appendlong(this_event_ties, 1, 0, WHITENULL_llll);
					active_cents_elem = active_cents_elem->l_next; 
					active_velocities_elem = active_velocities_elem->l_next; 
					active_slots_elem = active_slots_elem->l_next; 
					active_graphic_elem = active_graphic_elem->l_next; 
					active_until_elem = active_until_elem->l_next;
				}
			}
			tmp_chord = chord;
			while (tmp_chord) {
				for (note = tmp_chord->firstnote; note; note = note->next){
					llll_appenddouble(this_event_cents, note->midicents, 0, WHITENULL_llll);
					llll_appendlong(this_event_velocities, note->velocity, 0, WHITENULL_llll);
					llll_appendllll(this_event_graphic, note_get_graphic_values_no_router_as_llll((t_notation_obj *) x, note), 0, WHITENULL_llll);
					llll_appendllll(this_event_breakpoints, note_get_breakpoints_values_no_router_as_llll((t_notation_obj *) x, note), 0, WHITENULL_llll);
					llll_appendllll(this_event_slots, note_get_slots_values_no_header_as_llll((t_notation_obj *) x, note, false), 0, WHITENULL_llll);
					if (tmp_chord->onset + note->duration == next_onset) {
						llll_appendlong(this_event_ties, 0, 0, WHITENULL_llll);
					} else {
						llll_appendlong(this_event_ties, 1, 0, WHITENULL_llll);
						// we put the note into the active list
						llll_appenddouble(active_cents, note->midicents, 0, WHITENULL_llll);
						llll_appendlong(active_velocities, note->velocity, 0, WHITENULL_llll);
						llll_appendllll(active_graphic, note_get_graphic_values_no_router_as_llll((t_notation_obj *) x, note), 0, WHITENULL_llll);
						llll_appendllll(active_slots, note_get_slots_values_no_header_as_llll((t_notation_obj *) x, note, false), 0, WHITENULL_llll);
						llll_appenddouble(active_until, tmp_chord->onset + note->duration, 0, WHITENULL_llll);
					}
				}
				if (tmp_chord == lastchord_of_group)
					break;
				tmp_chord = tmp_chord->next;
			}

/*			char debug1[1000], debug2[1000], debug3[1000], debug4[1000], debug5[1000], debug6[1000];
			llll_to_char_array(this_event_cents, debug1, 999);
			llll_to_char_array(this_event_velocities, debug2, 999); 
			llll_to_char_array(this_event_ties, debug3, 999); 
			llll_to_char_array(this_event_graphic, debug4, 999); 
			llll_to_char_array(this_event_breakpoints, debug5, 999); 
			llll_to_char_array(this_event_slots, debug6, 999); */
			
			llll_appendllll(out_voice_cents, this_event_cents, 0, WHITENULL_llll); 
			llll_appenddouble(out_voice_durations, next_onset - curr_onset, 0, WHITENULL_llll); 
			llll_appendllll(out_voice_velocities, this_event_velocities, 0, WHITENULL_llll); 
			llll_appendllll(out_voice_ties, this_event_ties, 0, WHITENULL_llll); 
			llll_appendllll(out_voice_graphic, this_event_graphic, 0, WHITENULL_llll); 
			llll_appendllll(out_voice_breakpoints, this_event_breakpoints, 0, WHITENULL_llll); 
			llll_appendllll(out_voice_slots, this_event_slots, 0, WHITENULL_llll); 
			curr_onset = next_onset;
			
			// we do the same for all the middle-events (if any)
			while (active_until->l_size > 0) {
				t_llll *this_middle_event_cents, *this_middle_event_velocities, *this_middle_event_ties, *this_middle_event_graphic, *this_middle_event_breakpoints, *this_middle_event_slots;
				char then_break = false;
				
				// first: we find the next event start among the active events
				next_onset = -1;
				for (active_until_elem = active_until->l_head; active_until_elem; active_until_elem = active_until_elem->l_next) {
					double this_onset = hatom_getdouble(&active_until_elem->l_hatom);
					if (next_onset < 0 || this_onset < next_onset)
						next_onset = this_onset;
				}

				if (lastchord_of_group->next && next_onset >= lastchord_of_group->next->onset) {
					next_onset = lastchord_of_group->next->onset;
					if (curr_onset >= next_onset)
						break;
					else
						then_break = true;
				}
				
				// else: we build a new event
				this_middle_event_cents = llll_get();
				this_middle_event_velocities = llll_get();
				this_middle_event_ties = llll_get();
				this_middle_event_graphic = llll_get();
				this_middle_event_breakpoints = llll_get();
				this_middle_event_slots = llll_get();
				active_cents_elem = active_cents->l_head; 
				active_velocities_elem = active_velocities->l_head; 
				active_graphic_elem = active_graphic->l_head; 
				active_slots_elem = active_slots->l_head; 
				active_until_elem = active_until->l_head;
				while (active_cents_elem && active_velocities_elem && active_graphic_elem && active_slots_elem && active_until_elem) {
					double end_of_this_event = hatom_getdouble(&active_until_elem->l_hatom);
					llll_appenddouble(this_middle_event_cents, hatom_getdouble(&active_cents_elem->l_hatom), 0, WHITENULL_llll);
					llll_appendlong(this_middle_event_velocities, hatom_getdouble(&active_velocities_elem->l_hatom), 0, WHITENULL_llll);
					llll_appendhatom_clone(this_middle_event_graphic, &active_graphic_elem->l_hatom, 0, WHITENULL_llll);
					llll_appendllll(this_middle_event_breakpoints, llll_get(), 0, WHITENULL_llll);
					llll_appendhatom_clone(this_middle_event_slots, &active_slots_elem->l_hatom, 0, WHITENULL_llll);
					if (end_of_this_event == next_onset) {
						llll_appendlong(this_middle_event_ties, 0, 0, WHITENULL_llll);
						// the event is no more active!
						temp1 = active_cents_elem->l_next;
						temp2 = active_velocities_elem->l_next;
						temp3 = active_graphic_elem->l_next;
						temp4 = active_until_elem->l_next;
						temp5 = active_slots_elem->l_next;
						llll_destroyelem(active_cents_elem);
						llll_destroyelem(active_velocities_elem);
						llll_destroyelem(active_graphic_elem);
						llll_destroyelem(active_until_elem);
						llll_destroyelem(active_slots_elem);
						active_cents_elem = temp1; 
						active_velocities_elem = temp2; 
						active_graphic_elem = temp3; 
						active_until_elem = temp4;
						active_slots_elem = temp5;
					} else {
						llll_appendlong(this_middle_event_ties, 1, 0, WHITENULL_llll);
						active_cents_elem = active_cents_elem->l_next; 
						active_velocities_elem = active_velocities_elem->l_next; 
						active_graphic_elem = active_graphic_elem->l_next;
						active_slots_elem = active_slots_elem->l_next;
						active_until_elem = active_until_elem->l_next;
					}
				}

/*				llll_to_char_array(this_middle_event_cents, debug1, 999);
				llll_to_char_array(this_middle_event_velocities, debug2, 999); 
				llll_to_char_array(this_middle_event_ties, debug3, 999); 
				llll_to_char_array(this_middle_event_graphic, debug4, 999); 
				llll_to_char_array(this_middle_event_breakpoints, debug5, 999); 
				llll_to_char_array(this_middle_event_slots, debug6, 999); */
				
				llll_appendllll(out_voice_cents, this_middle_event_cents, 0, WHITENULL_llll); 
				llll_appenddouble(out_voice_durations, next_onset - curr_onset, 0, WHITENULL_llll); 
				llll_appendllll(out_voice_velocities, this_middle_event_velocities, 0, WHITENULL_llll); 
				llll_appendllll(out_voice_ties, this_middle_event_ties, 0, WHITENULL_llll); 
				llll_appendllll(out_voice_graphic, this_middle_event_graphic, 0, WHITENULL_llll); 
				llll_appendllll(out_voice_breakpoints, this_middle_event_breakpoints, 0, WHITENULL_llll); 
				llll_appendllll(out_voice_slots, this_middle_event_slots, 0, WHITENULL_llll); 
				
				curr_onset = next_onset;
				
				if (then_break) break;
			}
			
			chord = lastchord_of_group;
			
			// is there some pause before the beginning of the next chord ?
			if (active_until->l_size == 0 && chord->next && chord->next->onset > curr_onset) {
				// add a pause
				llll_appendllll(out_voice_cents, llll_get(), 0, WHITENULL_llll); 
				llll_appenddouble(out_voice_durations, -(chord->next->onset - curr_onset), 0, WHITENULL_llll); // pause: negative
				llll_appendllll(out_voice_velocities, llll_get(), 0, WHITENULL_llll); 
				llll_appendllll(out_voice_ties, llll_get(), 0, WHITENULL_llll); 
				llll_appendllll(out_voice_graphic, llll_get(), 0, WHITENULL_llll); 
				llll_appendllll(out_voice_breakpoints, llll_get(), 0, WHITENULL_llll); 
				llll_appendllll(out_voice_slots, llll_get(), 0, WHITENULL_llll); 
			}
			
		}

		llll_appendllll(out_cents, out_voice_cents, 0, WHITENULL_llll);
		llll_appendllll(out_durations, out_voice_durations, 0, WHITENULL_llll);
		llll_appendllll(out_velocities, out_voice_velocities, 0, WHITENULL_llll);
		llll_appendllll(out_ties, out_voice_ties, 0, WHITENULL_llll);
		llll_appendllll(out_graphic, out_voice_graphic, 0, WHITENULL_llll);
		llll_appendllll(out_breakpoints, out_voice_breakpoints, 0, WHITENULL_llll);
		llll_appendllll(out_slots, out_voice_slots, 0, WHITENULL_llll);

		llll_free(active_until);
		llll_free(active_velocities);
		llll_free(active_graphic);
		llll_free(active_slots);
		llll_free(active_cents);
	}
	
	// building extras
	llll_appendllll(out_extras, out_graphic, 0, WHITENULL_llll);	
	llll_appendllll(out_extras, out_breakpoints, 0, WHITENULL_llll);	
	llll_appendllll(out_extras, out_slots, 0, WHITENULL_llll);	
	
	what_to_dump_llll = llllobj_parse_llll((t_object *)x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_RETAIN);
	what_to_dump = header_objects_to_long(what_to_dump_llll);
	if (what_to_dump == 0)
		what_to_dump = k_HEADER_ALL;
	llll_free(what_to_dump_llll);
		
	llll_appendsym(out_llll, gensym("quantize"), 0, WHITENULL_llll);
	llll_chain(out_llll, get_notation_obj_header_as_llll((t_notation_obj *)x, what_to_dump, false, false, true, k_CONSIDER_FOR_DUMPING));
	llll_appendllll(out_llll, out_cents, 0, WHITENULL_llll);
	llll_appendllll(out_llll, out_durations, 0, WHITENULL_llll);
	llll_appendllll(out_llll, out_velocities, 0, WHITENULL_llll);
	llll_appendllll(out_llll, out_ties, 0, WHITENULL_llll);
	llll_appendllll(out_llll, out_extras, 0, WHITENULL_llll);
	
	unlock_general_mutex((t_notation_obj *)x);

	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 0, out_llll);
	llll_free(out_llll);
}

void roll_send_current_chord(t_roll *x){
	t_rollvoice *voice;
	t_chord *chord;
	t_note *note;
	t_llll *out_llll = llll_get();
	t_llll *out_cents = llll_get();
	t_llll *out_vels = llll_get();
	
	lock_general_mutex((t_notation_obj *)x);
	double curr_pos_ms = x->r_ob.playing ? x->r_ob.play_head_ms : x->r_ob.play_head_start_ms;
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
		for (chord = voice->firstchord; chord; chord = chord->next){
			if (chord->onset > curr_pos_ms)
				break;
			for (note = chord->firstnote; note; note = note->next){
				if (chord->onset <= curr_pos_ms && chord->onset + note->duration > curr_pos_ms){
					// breakpoints
					t_bpt *prev_bpt = note->firstbreakpoint;
					while (prev_bpt && prev_bpt->next && get_breakpoint_absolute_onset(prev_bpt->next) <= curr_pos_ms)
						prev_bpt = prev_bpt->next;
					
					if (!prev_bpt || !prev_bpt->next) {
						llll_appenddouble(out_cents, note->midicents + note->lastbreakpoint->delta_mc, 0, WHITENULL_llll);
						llll_appendlong(out_vels, x->r_ob.breakpoints_have_velocity ? note->lastbreakpoint->velocity : note->velocity, 0, WHITENULL_llll);
					} else {
						double cents = rescale_with_slope(curr_pos_ms, get_breakpoint_absolute_onset(prev_bpt), get_breakpoint_absolute_onset(prev_bpt->next), 
														  note->midicents + prev_bpt->delta_mc, note->midicents + prev_bpt->next->delta_mc, prev_bpt->next->slope, false);
						double velocity;
						if (x->r_ob.breakpoints_have_velocity)
							velocity = rescale(curr_pos_ms, get_breakpoint_absolute_onset(prev_bpt), get_breakpoint_absolute_onset(prev_bpt->next), 
											   prev_bpt->velocity, prev_bpt->next->velocity);
						else 
							velocity = note->velocity;
						llll_appenddouble(out_cents, cents, 0, WHITENULL_llll);
						llll_appendlong(out_vels, velocity, 0, WHITENULL_llll);
					}
				}
			}
		}
	}
	unlock_general_mutex((t_notation_obj *)x);
	
	llll_appendsym(out_llll, _llllobj_sym_currentchord, 0, WHITENULL_llll);
	llll_appendllll(out_llll, out_cents, 0, WHITENULL_llll);
	llll_appendllll(out_llll, out_vels, 0, WHITENULL_llll);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, out_llll);
	llll_free(out_llll);
}

// *******************************************************************************************
// cursor
// *******************************************************************************************

void roll_hidecursor(t_roll *x){
	x->r_ob.show_playhead = false;
	notationobj_redraw((t_notation_obj *) x);
}

void roll_showcursor(t_roll *x){
	x->r_ob.show_playhead = true;
    notationobj_redraw((t_notation_obj *) x);
}

void roll_setcursor(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	if (argc>0) {
		if (atom_gettype(argv) == A_SYM) {
			t_notation_item *it;
			t_llll *args = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_RETAIN);
			lock_general_mutex((t_notation_obj *)x);
			it = names_to_single_notation_item((t_notation_obj *) x, args);
			if (it)
				x->r_ob.play_head_start_ms  = notation_item_get_onset_ms((t_notation_obj *)x, it);
			unlock_general_mutex((t_notation_obj *)x);
			llll_free(args);
		} else if (is_atom_number(argv))
			x->r_ob.play_head_start_ms = atom_getfloat(argv);
	}
    if (x->r_ob.notify_also_upon_messages) {
        send_moved_playhead_position((t_notation_obj *) x, 6);
    }
    notationobj_redraw((t_notation_obj *) x);
}

void roll_getcursor(t_roll *x){
	send_playhead_position((t_notation_obj *) x, 6);
}


void roll_getloop(t_roll *x){
	send_loop_region((t_notation_obj *) x, 6);
}

// ********************************
// modifying selected objects
// *******************************


char are_any_chord_notes_locked(t_notation_obj *r_ob, t_chord *chord)
{
    t_note *note;
    for (note = chord->firstnote; note; note = note->next)
        if (note->locked)
            return true;
    return false;
}

char roll_sel_delete_item(t_roll *x, t_notation_item *curr_it, char *need_check_scheduling, t_llll *slots_to_transfer_to_next_note_in_chord_1based, char transfer_slots_even_if_empty)
{
	char changed = 0;
	if (curr_it->type == k_NOTE) {
		t_note *nt = (t_note *) curr_it;
        t_rollvoice *voice = nt->parent->voiceparent;
		notation_item_delete_from_selection((t_notation_obj *) x, curr_it);
		if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)){
			create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)nt, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
            transfer_note_slots((t_notation_obj *)x, nt, slots_to_transfer_to_next_note_in_chord_1based, transfer_slots_even_if_empty);
			note_delete((t_notation_obj *)x, nt, false);
            update_all_accidentals_for_voice_if_needed(x, voice);
			changed = 1;
		}
	} else if (curr_it->type == k_CHORD) {
		t_chord *ch = (t_chord *) curr_it;
        t_rollvoice *voice = ch->voiceparent;
		notation_item_delete_from_selection((t_notation_obj *) x, curr_it);
		if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)ch)){
            if (are_any_chord_notes_locked((t_notation_obj *)x, ch)) {
                t_note *nt;
                create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_UNDO_MODIFICATION_CHANGE);
                for (nt = ch->firstnote; nt; nt = nt->next)
                    if (!nt->locked)
                        note_delete((t_notation_obj *)x, nt, false);
                update_all_accidentals_for_voice_if_needed(x, voice);
                changed = 1;
            } else {
                create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_UNDO_MODIFICATION_ADD);
                if (delete_chord_from_voice((t_notation_obj *)x, ch, ch->prev, false)) {
                    if (need_check_scheduling) *need_check_scheduling = true;
                    update_all_accidentals_for_voice_if_needed(x, voice);
                }
            }
			changed = 1;
		}
	} else if (curr_it->type == k_PITCH_BREAKPOINT) {
		t_bpt *bpt = (t_bpt *) curr_it;
		notation_item_delete_from_selection((t_notation_obj *) x, curr_it);
		if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)bpt->owner)){
			create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)bpt->owner->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
			delete_breakpoint((t_notation_obj *) x, bpt);
			changed = 1;
		}
	} else if (curr_it->type == k_MARKER) {
		create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
		notation_item_delete_from_selection((t_notation_obj *) x, curr_it);
		delete_marker((t_notation_obj *) x, (t_marker *)curr_it);
		changed = 1;
	}	
	return changed;
}

void roll_delete_selection_and_transfer_default_slots(t_roll *x, char ripple)
{
    t_llll *slots_to_transfer = get_default_slots_to_transfer_1based((t_notation_obj *)x);
    roll_delete_selection(x, ripple, slots_to_transfer, false);
    llll_free(slots_to_transfer);
}


void roll_delete_selection(t_roll *x, char ripple, t_llll *slots_to_transfer_to_next_note_in_chord_1based, char transfer_slots_even_if_empty)
{
	t_notation_item *curr_it = x->r_ob.firstselecteditem;
	t_notation_item *curr_it2 = NULL;
	char changed = 0;
	char need_check_scheduling = false;
	t_notation_item *lambda_it = x->r_ob.lambda_selected_item_ID > 0 ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : NULL;
	
	lock_general_mutex((t_notation_obj *)x);
    
    double left_ms = ripple ? get_selection_leftmost_onset(x) : 0;
    double right_ms = ripple ? get_selection_rightmost_onset(x) : 0;
    double delta = left_ms - right_ms;

	for (curr_it = x->r_ob.firstselecteditem; curr_it; curr_it = curr_it2) {
		curr_it2 = curr_it->next_selected;
		
		if (lambda_it && (lambda_it == curr_it || // lambda item is exactly the item we're deleting..
						  notation_item_is_ancestor_of((t_notation_obj *)x, lambda_it, curr_it) || // or one of its ancestors...
						  notation_item_is_ancestor_of((t_notation_obj *)x, curr_it, lambda_it))) { // or one of its progeny...
//			cpost("Trying to delete item %p (type %ld). Can't.", curr_it, curr_it->type);
//			object_error((t_object *)x, "Can't delete item, it's being output from the playout!");
			curr_it->flags = k_FLAG_TO_BE_DELETED;
			continue;
		}
		
//		cpost("Delete item %p of type %ld", curr_it, curr_it->type);
        changed |= roll_sel_delete_item(x, curr_it, &need_check_scheduling, slots_to_transfer_to_next_note_in_chord_1based, transfer_slots_even_if_empty);
	}
	
	if (changed)
		recompute_total_length((t_notation_obj *)x);
	
	if (need_check_scheduling)
		check_correct_scheduling((t_notation_obj *)x, false);
	

    if (ripple) {
        preselect_notes_in_region((t_notation_obj *)x, right_ms, x->r_ob.length_ms + 1000, -320000, 36000, 0, x->r_ob.num_voices, true, true);
        preselect_markers_in_region((t_notation_obj *)x, right_ms, x->r_ob.length_ms + 1000);
        move_preselecteditems_to_selection((t_notation_obj *)x, k_SELECTION_MODE_FORCE_SELECT, false, false);
        unlock_general_mutex((t_notation_obj *)x);
        change_selection_onset(x, &delta);
        lock_general_mutex((t_notation_obj *)x);
        clear_selection((t_notation_obj *)x);
    }
    
	unlock_general_mutex((t_notation_obj *)x);
	
}

void roll_sel_ripple_delete(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    t_llll *transfer_slots = NULL;
    char even_if_empty = false;
    t_llll *ll = llllobj_parse_llll((t_object *)x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
    llll_parseargs_and_attrs_destructive((t_object *) x, ll, "li", gensym("transferslots"), &transfer_slots, gensym("empty"), &even_if_empty);
    llll_free(ll);
    
    roll_delete_selection(x, true, transfer_slots, even_if_empty);
    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_RIPPLE_DELETE_SELECTION);
}

void roll_sel_delete(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    t_llll *transfer_slots = NULL;
    char even_if_empty = false;
    t_llll *ll = llllobj_parse_llll((t_object *)x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
    llll_parseargs_and_attrs_destructive((t_object *) x, ll, "li", gensym("transferslots"), &transfer_slots, gensym("empty"), &even_if_empty);
    llll_free(ll);
    
    roll_delete_selection(x, false, transfer_slots, even_if_empty);
    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_DELETE_SELECTION);
}

void roll_clearmarkers(t_roll *x)
{
	create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
	lock_general_mutex((t_notation_obj *)x);
	lock_markers_mutex((t_notation_obj *)x);;
	clear_all_markers((t_notation_obj *) x);
	unlock_markers_mutex((t_notation_obj *)x);;
	unlock_general_mutex((t_notation_obj *)x);
	update_hscrollbar((t_notation_obj *)x, 0);
	
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CLEAR_MARKERS);
}

void roll_sel_snap_pitch_to_grid(t_roll *x){
	snap_pitch_to_grid_for_selection((t_notation_obj *) x);
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_SNAP_PITCH_TO_GRID_FOR_SELECTION);
}

char snap_onset_to_grid_for_selection(t_roll *x){ 
	if (x->r_ob.show_grid == 1 || x->r_ob.ruler > 0) { 
		char res = iterate_chordwise_changes_on_selection((t_notation_obj *)x, (notation_obj_chord_fn) snap_onset_to_grid_for_chord, NULL, true, k_CHORD, true);
        check_all_chords_order(x);
        
		recompute_total_length((t_notation_obj *)x);
		return res;
	}
	return 0;
}

char snap_markers_to_grid_for_selection(t_roll *x){
    if (x->r_ob.show_grid == 1 || x->r_ob.ruler > 0) {
        char res = 0;
        // markers
        t_marker *marker;
        for (marker = x->r_ob.firstmarker; marker; marker = marker->next)
            if (notation_item_is_selected((t_notation_obj *)x, (t_notation_item *)marker)) {
                res = 1;
                snap_onset_to_grid_for_marker((t_notation_obj *)x, marker, NULL);
            }
        check_markers_order((t_notation_obj *)x);
        
        recompute_total_length((t_notation_obj *)x);
        return res;
    }
    return 0;
}


char snap_to_grid_for_flagged_items(t_roll *x, char onset, char tail)
{
    if (!onset && !tail)
        return 0; // nothing to do
    
    char res = 0;
    if (x->r_ob.show_grid == 1 || x->r_ob.ruler > 0) {
        t_rollvoice *voice;
        t_chord *chord;
        for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next)
            for (chord = voice->firstchord; chord; chord = chord->next) {
                if (chord->r_it.flags & k_FLAG_TO_BE_SNAPPED) {
                    res = 1;
                    if (onset)
                        snap_onset_to_grid_for_chord((t_notation_obj *)x, chord, NULL);
                    if (tail) {
                        t_note *note;
                        for (note = chord->firstnote; note; note = note->next)
                            snap_tail_to_grid_for_note((t_notation_obj *)x, note, NULL);
                    }
                    chord->r_it.flags &= ~k_FLAG_TO_BE_SNAPPED;
                }
            }
        check_all_chords_order(x);
        
        // markers
        t_marker *marker;
        if (onset) {
            for (marker = x->r_ob.firstmarker; marker; marker = marker->next)
                if (marker->r_it.flags & k_FLAG_TO_BE_SNAPPED) {
                    res = 1;
                    snap_onset_to_grid_for_marker((t_notation_obj *)x, marker, NULL);
                    marker->r_it.flags &= ~k_FLAG_TO_BE_SNAPPED;
                }
            check_markers_order((t_notation_obj *)x);
        }
        recompute_total_length((t_notation_obj *)x);
        return res;
    }
    return 0;
}


void roll_sel_snap_onset_to_grid(t_roll *x)
{
	snap_onset_to_grid_for_selection(x);
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_SNAP_ONSET_TO_GRID_FOR_SELECTION);
}

void roll_sel_snap_tail_to_grid(t_roll *x)
{
	snap_tail_to_grid_for_selection((t_notation_obj *) x);
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_SNAP_TAIL_TO_GRID_FOR_SELECTION);
}


void roll_explodechords(t_roll *x, char selection_only)
{
	t_rollvoice *voice;
//	char changed = false;
	create_whole_roll_undo_tick(x);
	lock_general_mutex((t_notation_obj *)x);

	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
		t_chord *chord;
		for (chord = voice->firstchord; chord; chord = chord->next)
            if (!selection_only || notation_item_is_globally_selected((t_notation_obj *)x, (t_notation_item *)chord))
                chord->r_it.flags = (e_bach_internal_notation_flags) (chord->r_it.flags | k_FLAG_TO_BE_MODIFIED);
	}

	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
		t_chord *chord = voice->firstchord;
		while (chord) {
				t_chord *nextchord = chord->next;
			
			if (chord->r_it.flags & k_FLAG_TO_BE_MODIFIED) {
				double onset = chord->onset;
				t_note *note = chord->firstnote;
				while (note) {
					t_note *nextnote = note->next;
					t_note *clonednote = clone_note((t_notation_obj *)x, note, k_CLONE_FOR_ORIGINAL);
					addchord_from_notes(x, voice->v_ob.number, onset, -1, 1, clonednote, clonednote, false, 0);
					note = nextnote;
				}
				delete_chord_from_voice((t_notation_obj *)x, chord, NULL, false);
			}
			
			chord = nextchord;
		}
	}
	
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
		t_chord *chord;
		for (chord = voice->firstchord; chord; chord = chord->next)
			chord->r_it.flags = (e_bach_internal_notation_flags) (chord->r_it.flags & ~k_FLAG_TO_BE_MODIFIED);
	}
	
	check_correct_scheduling((t_notation_obj *)x, false);
	unlock_general_mutex((t_notation_obj *)x);
	
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_RESET_ALL_ENHARMONICITIES);
}

void roll_resetgraphic(t_roll *x){
	t_rollvoice *voice;
	char changed = false;
	lock_general_mutex((t_notation_obj *)x);
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
		t_chord *chord;
		for (chord = voice->firstchord; chord; chord = chord->next){
			t_note *note;
			create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)chord, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
			for (note = chord->firstnote; note; note = note->next){
				changed |= reset_note_graphic((t_notation_obj *) x, note);
			}
		}
	}
	unlock_general_mutex((t_notation_obj *)x);
	
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_RESET_ALL_ENHARMONICITIES);
}

void roll_sel_resetgraphic(t_roll *x){
	char changed;
	changed = reset_selection_graphic((t_notation_obj *) x);
    update_all_accidentals_if_needed(x);
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_RESET_ENHARMONICITY_FOR_SELECTION);
}

void select_notes_with_lexpr(t_roll *x, e_selection_modes mode)
{
	t_rollvoice *voice;
	lock_general_mutex((t_notation_obj *)x);
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
		t_chord *chord;
		for (chord = voice->firstchord; chord; chord = chord->next){
			t_note *note;
			for (note = chord->firstnote; note; note = note->next){
				t_hatom *res = lexpr_eval_for_notation_item((t_notation_obj *)x, (t_notation_item *)note, x->r_ob.n_lexpr);
				if (hatom_gettype(res) == H_LONG && hatom_getlong(res) != 0) 
					notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)note);
				bach_freeptr(res);
			}
		}
	}
	move_preselecteditems_to_selection((t_notation_obj *)x, mode, false, false);
	unlock_general_mutex((t_notation_obj *)x);
}

void roll_clearnames(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	t_llll *args = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_RETAIN);
	long voices = 1, chords = 1, notes = 1, markers = 1;
//	llll_parseargs((t_object *)x, args, "iiiii", _llllobj_sym_markers, &markers, _llllobj_sym_voices, &voices, _llllobj_sym_chords, &chords, _llllobj_sym_notes, &notes);
    voices = (args->l_size == 0 || is_symbol_in_llll_first_level(args, _llllobj_sym_voices));
    chords = (args->l_size == 0 || is_symbol_in_llll_first_level(args, _llllobj_sym_chords));
    notes = (args->l_size == 0 || is_symbol_in_llll_first_level(args, _llllobj_sym_notes));
    markers = (args->l_size == 0 || is_symbol_in_llll_first_level(args, _llllobj_sym_markers));
    
	notation_obj_clear_names((t_notation_obj *)x, voices, false, chords, notes, markers);
	llll_free(args);
}

void roll_name(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	notation_obj_name((t_notation_obj *)x, s, argc, argv);
}


void roll_nameappend(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	notation_obj_nameappend((t_notation_obj *)x, s, argc, argv);
}

void roll_slottoname(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    notation_obj_slottoname((t_notation_obj *)x, s, argc, argv);
}

void roll_nametoslot(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    notation_obj_nametoslot((t_notation_obj *)x, s, argc, argv);
}



void roll_role(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    notation_obj_role((t_notation_obj *)x, s, argc, argv);
}


void roll_group(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	if (x->r_ob.firstselecteditem){
        create_header_undo_tick((t_notation_obj *)x, k_HEADER_GROUPS);
        
        lock_general_mutex((t_notation_obj *)x);
        build_and_append_group_from_selection((t_notation_obj *) x);
        unlock_general_mutex((t_notation_obj *)x);

        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CREATE_GROUP);
    }
}


void roll_ungroup(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    if (x->r_ob.firstselecteditem){
        t_llll *all_groups = llll_get();
        
        create_header_undo_tick((t_notation_obj *)x, k_HEADER_GROUPS);
        
        lock_general_mutex((t_notation_obj *)x);
        
        // retrieve all groups
        for (t_notation_item *item = x->r_ob.firstselecteditem; item; item = item->next_selected){
            if (item->type == k_CHORD) {
                t_chord *thisch = (t_chord *)item;
                if (thisch->r_it.group)
                    llll_appendobj(all_groups, thisch->r_it.group);
            }
        }
        
        all_groups = llll_thin_simple(all_groups, true);
        
        for (t_llllelem *elem = all_groups->l_head; elem; elem = elem->l_next) {
            t_group *gr = (t_group *)hatom_getobj(&elem->l_hatom);
                delete_group((t_notation_obj *) x, gr);
        }
        
        unlock_general_mutex((t_notation_obj *)x);

        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_GROUP);
        llll_free(all_groups);
    }
}




void roll_select(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	t_llll *selectllll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_RETAIN);
	e_selection_modes mode = symbol_to_mode((t_notation_obj *)x, s);
	
	if (selectllll && selectllll->l_size > 0) {
		long head_type = hatom_gettype(&selectllll->l_head->l_hatom);
		
		// (un)sel(ect) chord
		if (head_type == H_SYM && hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_chord && selectllll->l_head->l_next) {
			t_chord *to_select;
			lock_general_mutex((t_notation_obj *)x);
			if (selectllll->l_depth == 1) {
				if ((to_select = chord_get_from_path_as_llllelem_range((t_notation_obj *)x, selectllll->l_head->l_next)))
					add_all_chord_notes_to_preselection((t_notation_obj *)x, to_select);
			} else {
				t_llllelem *elem;
				for (elem = selectllll->l_head->l_next; elem; elem = elem->l_next) 
					if (hatom_gettype(&elem->l_hatom) == H_LLLL)
						if ((to_select = chord_get_from_path_as_llllelem_range((t_notation_obj *)x, hatom_getllll(&elem->l_hatom)->l_head)))
							add_all_chord_notes_to_preselection((t_notation_obj *)x, to_select);
			}
			move_preselecteditems_to_selection((t_notation_obj *) x, mode, false, false);
			unlock_general_mutex((t_notation_obj *)x);
			
        // (un)sel(ect) breakpoint/tail if
        } else if (head_type == H_SYM && (hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_breakpoint || hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_tail) && selectllll->l_head->l_next &&
                   hatom_gettype(&selectllll->l_head->l_next->l_hatom) == H_SYM &&
                   hatom_getsym(&selectllll->l_head->l_next->l_hatom) == _llllobj_sym_if) {
            
            char tail_only = (hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_tail);
            t_atom *new_av = NULL;
            long new_ac = 0;
            
            if (x->r_ob.n_lexpr)
                lexpr_free(x->r_ob.n_lexpr);
            
            llll_behead(selectllll);
            llll_behead(selectllll);
            
            new_ac = llll_deparse(selectllll, &new_av, 0, 0);
            x->r_ob.n_lexpr = notation_obj_lexpr_new(new_ac, new_av);
            
            if (new_av)
                bach_freeptr(new_av);
            
            if (x->r_ob.n_lexpr) {
                select_breakpoints_with_lexpr((t_notation_obj *)x, mode, tail_only);
            } else {
                object_error((t_object *) x, "Bad expression!");
            }

        // (un)sel(ect) marker if
        } else if (head_type == H_SYM && hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_marker && selectllll->l_head->l_next &&
                   hatom_gettype(&selectllll->l_head->l_next->l_hatom) == H_SYM &&
                   hatom_getsym(&selectllll->l_head->l_next->l_hatom) == _llllobj_sym_if) {

            t_atom *new_av = NULL;
            long new_ac = 0;
            
            if (x->r_ob.n_lexpr)
                lexpr_free(x->r_ob.n_lexpr);
            
            llll_behead(selectllll);
            llll_behead(selectllll);
            
            new_ac = llll_deparse(selectllll, &new_av, 0, 0);
            x->r_ob.n_lexpr = notation_obj_lexpr_new(new_ac, new_av);
            
            if (new_av)
                bach_freeptr(new_av);
            
            if (x->r_ob.n_lexpr) {
                select_markers_with_lexpr((t_notation_obj *)x, mode);
            } else {
                object_error((t_object *) x, "Bad expression!");
            }

        // (un)sel(ect) marker by index
        } else if (head_type == H_SYM && hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_marker && selectllll->l_head->l_next) {
            t_marker *to_select;
            lock_general_mutex((t_notation_obj *)x);
            if (selectllll->l_depth == 1) {
                if ((to_select = get_marker_from_path_as_llllelem_range((t_notation_obj *)x, selectllll->l_head->l_next)))
                    notation_item_add_to_preselection((t_notation_obj *)x, (t_notation_item *)to_select);
            } else {
                t_llllelem *elem;
                for (elem = selectllll->l_head->l_next; elem; elem = elem->l_next)
                    if (hatom_gettype(&elem->l_hatom) == H_LLLL)
                        if ((to_select = get_marker_from_path_as_llllelem_range((t_notation_obj *)x, hatom_getllll(&elem->l_hatom)->l_head)))
                            notation_item_add_to_preselection((t_notation_obj *)x, (t_notation_item *)to_select);
            }
            move_preselecteditems_to_selection((t_notation_obj *) x, mode, false, false);
            unlock_general_mutex((t_notation_obj *)x);
            
        // (un)sel(ect) markers
		} else if (head_type == H_SYM && hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_markers) {
			select_all_markers((t_notation_obj *)x, mode);

        // (un)sel(ect) all notes
        } else if (head_type == H_SYM && (hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_notes || hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_chords)) {
            select_all_chords((t_notation_obj *)x, mode);

        // (un)sel(ect) all breakpoints
        } else if (head_type == H_SYM && hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_breakpoints) {
            select_all_breakpoints((t_notation_obj *)x, mode, false);

            // (un)sel(ect) all tails
        } else if (head_type == H_SYM && hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_tails) {
            select_all_breakpoints((t_notation_obj *)x, mode, true);

		// (un)sel(ect) all
		} else if (head_type == H_SYM && hatom_getsym(&selectllll->l_head->l_hatom) == _sym_all) {
			if (mode == k_SELECTION_MODE_FORCE_UNSELECT)
				clear_selection((t_notation_obj *)x);
			else
				select_all(x);
			
		// (un)sel(ect) note if
		} else if (head_type == H_SYM && hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_note && selectllll->l_head->l_next && 
				   hatom_gettype(&selectllll->l_head->l_next->l_hatom) == H_SYM &&
				   hatom_getsym(&selectllll->l_head->l_next->l_hatom) == _llllobj_sym_if) {
			
			t_atom *new_av = NULL; 
			long new_ac = 0;
			
			if (x->r_ob.n_lexpr) 
				lexpr_free(x->r_ob.n_lexpr);
			
			llll_behead(selectllll);
			llll_behead(selectllll);
			
			new_ac = llll_deparse(selectllll, &new_av, 0, 0);
			x->r_ob.n_lexpr = notation_obj_lexpr_new(new_ac, new_av);
			
			if (new_av) 
				bach_freeptr(new_av);
			
			if (x->r_ob.n_lexpr) {
				select_notes_with_lexpr(x, mode);
			} else {
				object_error((t_object *) x, "Bad expression!");
			}

		// (un)sel(ect) note
		} else if (head_type == H_SYM && hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_note && selectllll->l_head->l_next) {

			t_note *to_select;
			lock_general_mutex((t_notation_obj *)x);
			if (selectllll->l_depth == 1) {
				if ((to_select = note_get_from_path_as_llllelem_range((t_notation_obj *)x, selectllll->l_head->l_next)))
					notation_item_add_to_preselection((t_notation_obj *)x, (t_notation_item *)to_select);
			} else {
				t_llllelem *elem;
				for (elem = selectllll->l_head->l_next; elem; elem = elem->l_next) 
					if (hatom_gettype(&elem->l_hatom) == H_LLLL)
						if ((to_select = note_get_from_path_as_llllelem_range((t_notation_obj *)x, hatom_getllll(&elem->l_hatom)->l_head)))
							notation_item_add_to_preselection((t_notation_obj *)x, (t_notation_item *)to_select);
			}
			move_preselecteditems_to_selection((t_notation_obj *) x, mode, false, false);
			unlock_general_mutex((t_notation_obj *)x);

        // warn: maybe they meant goto!
        } else if (head_type == H_SYM && (hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_next || hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_prev || hatom_getsym(&selectllll->l_head->l_hatom) == _llllobj_sym_time)) {
            object_warn((t_object *)x, "Unknown selection mode '%s'", hatom_getsym(&selectllll->l_head->l_hatom)->s_name);
            object_warn((t_object *)x, "    Maybe you meant to use it as command for the 'goto' message?");
            
		// (un)sel(ect) by name
		} else if (head_type == H_SYM || selectllll->l_size == 1) {
			lock_markers_mutex((t_notation_obj *)x);;
			preselect_notation_items_by_name((t_notation_obj *)x, selectllll);
			move_preselecteditems_to_selection((t_notation_obj *) x, mode, false, false);
			unlock_markers_mutex((t_notation_obj *)x);;

		// (un)sel(ect) by rectangle
		} else {
			
			// select all the elements within a given region
			double ms1, ms2, mc1, mc2;
			ms1 = ((selectllll->l_size >= 1) && is_hatom_number(&selectllll->l_head->l_hatom)) ? hatom_getdouble(&selectllll->l_head->l_hatom) : 0.; 
			if (ms1 < 0.) 
				ms1 = 0.; 
			else 
				ms1 -= CONST_EPSILON_SELECT;
			ms2 = ((selectllll->l_size >= 2) && is_hatom_number(&selectllll->l_head->l_next->l_hatom)) ? hatom_getdouble(&selectllll->l_head->l_next->l_hatom) : x->r_ob.length_ms; 
			if (ms2 < 0.) 
				ms2 = x->r_ob.length_ms; 
			else 
				ms2 += CONST_EPSILON_SELECT;
			if (ms1 > ms2) 
				return;
			
			mc1 = ((selectllll->l_size >= 3) && is_hatom_number(&selectllll->l_head->l_next->l_next->l_hatom)) ? 
			hatom_getdouble(&selectllll->l_head->l_next->l_next->l_hatom) : -16000; 
			mc1 -= CONST_EPSILON_SELECT;
			mc2 = ((selectllll->l_size >= 4) && is_hatom_number(&selectllll->l_head->l_next->l_next->l_next->l_hatom)) ? 
			hatom_getdouble(&selectllll->l_head->l_next->l_next->l_next->l_hatom) : 32000; 
			mc2 += CONST_EPSILON_SELECT;
			if (mc1 > mc2) 
				return;
			
			
			t_llll *voice_numbers = NULL; // will be 0-based
			if (selectllll->l_size >= 5) {
				t_llllelem *voiceelem = selectllll->l_head->l_next->l_next->l_next->l_next; 
				if (hatom_gettype(&voiceelem->l_hatom) == H_LLLL) {
					voice_numbers = llll_clone(hatom_getllll(&voiceelem->l_hatom));
					llll_develop_ranges_and_parse_negative_indices_inplace(&voice_numbers, x->r_ob.num_voices, true);
				} else if (hatom_gettype(&voiceelem->l_hatom) == H_LONG) {
					voice_numbers = llll_get();
					llll_appendlong(voice_numbers, hatom_getlong(&voiceelem->l_hatom) - 1, 0, WHITENULL_llll);
				}
			}
			
			lock_general_mutex((t_notation_obj *)x);
			t_rollvoice *voice;
			clear_preselection((t_notation_obj *)x);
			if (!voice_numbers || voice_numbers->l_size == 0) {
				for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next)
					preselect_elements_in_region_for_mouse_selection(x, ms1, ms2, mc1, mc2, voice->v_ob.number, voice->v_ob.number);
			} else {
				t_llllelem *elem;
				for (elem = voice_numbers->l_head; elem; elem = elem->l_next)
					if (hatom_gettype(&elem->l_hatom) == H_LONG) {
						long this_voice_num = hatom_getlong(&elem->l_hatom);
						preselect_elements_in_region_for_mouse_selection(x, ms1, ms2, mc1, mc2, this_voice_num, this_voice_num);
					}
			}
			preselect_markers_in_region((t_notation_obj *)x, ms1, ms2);
			move_preselecteditems_to_selection((t_notation_obj *) x, mode, false, false);
			unlock_general_mutex((t_notation_obj *)x);
		}
		handle_change_selection((t_notation_obj *)x);
	}
	
	if (selectllll) 
		llll_free(selectllll);

	invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
}

void roll_sel_change_onset(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	t_llll *new_onset = NULL;
	t_lexpr *lexpr = NULL;
	t_notation_item *curr_it;
	char changed = 0;
	char lambda = (s == _llllobj_sym_lambda);

	if (argc <= 0) 
		return;
	
	if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("="))
		lexpr = notation_obj_lexpr_new(argc - 1, argv + 1);
	else
		new_onset = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
	
	lock_general_mutex((t_notation_obj *)x);
	curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
	while (curr_it) {
        switch (curr_it->type) {
            case k_NOTE:
                changed |= change_chord_onset_from_lexpr_or_llll((t_notation_obj *)x, ((t_note *) curr_it)->parent, lexpr, new_onset);
                update_all_accidentals_for_chord_if_needed(x, ((t_note *) curr_it)->parent);
                break;
                
            case k_CHORD:
                changed |= change_chord_onset_from_lexpr_or_llll((t_notation_obj *)x, (t_chord *) curr_it, lexpr, new_onset);
                update_all_accidentals_for_chord_if_needed(x, (t_chord *) curr_it);
                break;

            case k_PITCH_BREAKPOINT:
                changed |= change_breakpoint_onset_from_lexpr_or_llll((t_notation_obj *)x, (t_bpt *) curr_it, lexpr, new_onset);
                break;

            case k_MARKER:
            {
                t_marker *marker = (t_marker *) curr_it;
                double ms = marker->position_ms;
                create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
                change_double((t_notation_obj *)x, &ms, lexpr, new_onset ? new_onset->l_head : NULL, 0, curr_it);
                change_marker_ms((t_notation_obj *) x, marker, ms, 0, 0);
                changed = 1;
            }
                break;
                
            default:
                break;
        }

        curr_it = lambda ? NULL : curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);

	if (new_onset) 
		llll_free(new_onset);
	if (lexpr)
		lexpr_free(lexpr);

	if (changed) {
		lock_general_mutex((t_notation_obj *)x);
		check_all_chords_order(x);
		unlock_general_mutex((t_notation_obj *)x);
		recompute_total_length((t_notation_obj *)x);
		handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_ONSET_FOR_SELECTION);
	}
}

// undocumented, because troublesome.
// should be done via scrisp in a foreseable future as onset@(chordindex+1) = onset@chordindex + 100
void roll_sel_change_ioi(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    t_llll *new_ioi = NULL;
    t_lexpr *lexpr = NULL;
    t_notation_item *curr_it;
    char changed = 0;
    char lambda = (s == _llllobj_sym_lambda);
    
    if (argc <= 0)
        return;
    
    if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("="))
        lexpr = notation_obj_lexpr_new(argc - 1, argv + 1);
    else
        new_ioi = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
    
    lock_general_mutex((t_notation_obj *)x);
    curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
    while (curr_it) {
        switch (curr_it->type) {
            case k_NOTE:
                changed |= change_chord_ioi_from_lexpr_or_llll((t_notation_obj *)x, ((t_note *) curr_it)->parent, lexpr, new_ioi);
                update_all_accidentals_for_chord_if_needed(x, ((t_note *) curr_it)->parent);
                break;
                
            case k_CHORD:
                changed |= change_chord_ioi_from_lexpr_or_llll((t_notation_obj *)x, (t_chord *) curr_it, lexpr, new_ioi);
                update_all_accidentals_for_chord_if_needed(x, (t_chord *) curr_it);
                break;
                
/*            case k_PITCH_BREAKPOINT:
                changed |= change_breakpoint_onset_from_lexpr_or_llll((t_notation_obj *)x, (t_bpt *) curr_it, lexpr, new_onset);
                break;
                
            case k_MARKER:
            {
                t_marker *marker = (t_marker *) curr_it;
                double ms = marker->position_ms;
                create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
                change_double((t_notation_obj *)x, &ms, lexpr, new_onset ? new_onset->l_head : NULL, 0, curr_it);
                change_marker_ms((t_notation_obj *) x, marker, ms, 0, 0);
                changed = 1;
            }
                break;
                */
            default:
                break;
        }
        
        curr_it = lambda ? NULL : curr_it->next_selected;
    }
    unlock_general_mutex((t_notation_obj *)x);
    
    if (new_ioi)
        llll_free(new_ioi);
    if (lexpr)
        lexpr_free(lexpr);
    
    if (changed) {
        lock_general_mutex((t_notation_obj *)x);
        check_all_chords_order(x);
        unlock_general_mutex((t_notation_obj *)x);
        recompute_total_length((t_notation_obj *)x);
        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_IOI_FOR_SELECTION);
    }
}

void roll_sel_change_duration(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_lexpr *lexpr = NULL;
	t_llll *new_duration = NULL;
	t_notation_item *curr_it;
	char changed = 0;
	char lambda = (s == _llllobj_sym_lambda);

	if (argc <= 0) 
		return;

	if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("="))
		lexpr = notation_obj_lexpr_new(argc - 1, argv + 1);
	else
		new_duration = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
	
	lock_general_mutex((t_notation_obj *)x);
	curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
	while (curr_it) {
		if (curr_it->type == k_NOTE) {
			changed |= change_note_duration_from_lexpr_or_llll((t_notation_obj *)x, (t_note *) curr_it, lexpr, new_duration);
		} else if (curr_it->type == k_CHORD) {
			changed |= change_chord_duration_from_lexpr_or_llll((t_notation_obj *)x, (t_chord *) curr_it, lexpr, new_duration);
		}
		curr_it = lambda ? NULL : curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);
	
	if (new_duration) 
		llll_free(new_duration);
	if (lexpr)
		lexpr_free(lexpr);

	if (changed) {
		recompute_total_length((t_notation_obj *)x);
		handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_DURATION_FOR_SELECTION);
	}
}

// mode 0 = standard, mode -1 = trim, mode 1 = extend
char legato(t_roll *x, long mode)
{
	t_atom newav[1];
    atom_setsym(newav, (mode > 0 ? _llllobj_sym_legatoextend : (mode < 0 ? _llllobj_sym_legatotrim : _llllobj_sym_legato)));
	return roll_do_sel_change_tail(x, NULL, 1, newav);
}

// mode 0 = standard, mode -1 = trim, mode 1 = extend
char glissando(t_roll *x, long mode, double slope)
{
    t_atom newav[2];
    atom_setsym(newav, (mode > 0 ? _llllobj_sym_glissandoextend : (mode < 0 ? _llllobj_sym_glissandotrim : _llllobj_sym_glissando)));
    atom_setfloat(newav+1, slope);
    return roll_do_sel_change_tail(x, NULL, 2, newav);
}

void roll_legato(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    long mode = 0;
    if (argc && argv && atom_gettype(argv) == A_SYM) {
        if (atom_getsym(argv) == gensym("extend"))
            mode = 1;
        else if (atom_getsym(argv) == gensym("trim"))
            mode = -1;
    }
	if (legato(x, mode))
		handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_LEGATO_FOR_SELECTION);
}

void roll_glissando(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    long mode = 0;
    double slope = 0;
    if (argc && argv) {
        if (atom_gettype(argv) == A_SYM) {
            if (atom_getsym(argv) == gensym("extend"))
                mode = 1;
            else if (atom_getsym(argv) == gensym("trim"))
                mode = -1;
            if (argc > 1 && is_atom_number(argv) + 1)
                slope = atom_getfloat(argv + 1);
        } else if (is_atom_number(argv))
            slope = atom_getfloat(argv);
    }
    
    if (glissando(x, mode, slope))
        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_GLISSANDO_FOR_SELECTION);
}

void roll_sel_change_tail(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	if (roll_do_sel_change_tail(x, s, argc, argv))
		handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_TAIL_FOR_SELECTION);
}

char roll_do_sel_change_tail(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_lexpr *lexpr = NULL;
	t_llll *new_tail = NULL;
	t_notation_item *curr_it;
	char changed = 0;
	char lambda = (s == _llllobj_sym_lambda);
	
	if (argc <= 0) 
		return 0;
	
	if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("="))
		lexpr = notation_obj_lexpr_new(argc - 1, argv + 1);
	else
		new_tail = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
	
	lock_general_mutex((t_notation_obj *)x);
	curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;

	while (curr_it) {
		if (curr_it->type == k_NOTE) {
			changed |= change_note_tail_from_lexpr_or_llll((t_notation_obj *)x, (t_note *) curr_it, lexpr, new_tail);
		} else if (curr_it->type == k_CHORD) {
			changed |= change_chord_tail_from_lexpr_or_llll((t_notation_obj *)x, (t_chord *) curr_it, lexpr, new_tail);
		}
		curr_it = lambda ? NULL : curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);
	
	if (new_tail) 
		llll_free(new_tail);
	if (lexpr)
		lexpr_free(lexpr);

	if (changed)
		recompute_total_length((t_notation_obj *)x);

	return changed;
}

void roll_sel_change_cents(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_lexpr *lexpr = NULL;
	t_llll *new_cents = NULL;
	t_notation_item *curr_it;
	char changed = 0;
	char lambda = (s == _llllobj_sym_lambda);
	
	if (argc <= 0) return;

	if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("="))
		lexpr = notation_obj_lexpr_new(argc - 1, argv + 1);
	else
		new_cents = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);

	lock_general_mutex((t_notation_obj *)x);
	curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
	while (curr_it) {
		if (curr_it->type == k_NOTE) {
			changed |= change_note_cents_from_lexpr_or_llll((t_notation_obj *)x, (t_note *) curr_it, lexpr, new_cents);
		} else if (curr_it->type == k_CHORD) {
			changed |= change_chord_cents_from_lexpr_or_llll((t_notation_obj *)x, (t_chord *) curr_it, lexpr, new_cents);
        } else if (curr_it->type == k_PITCH_BREAKPOINT) {
            changed |= change_breakpoint_cents_from_lexpr_or_llll((t_notation_obj *)x, (t_bpt *) curr_it, lexpr, new_cents);
		}
		curr_it = lambda ? NULL : curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);

	if (new_cents) 
		llll_free(new_cents);
	if (lexpr)
		lexpr_free(lexpr);

	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_CENTS_FOR_SELECTION);
}

void roll_sel_change_pitch(t_roll *x, t_symbol *s, long argc, t_atom *argv){
    t_lexpr *lexpr = NULL;
    t_llll *new_pitch = NULL;
    t_notation_item *curr_it;
    char changed = 0;
    char lambda = (s == _llllobj_sym_lambda);
    
    if (argc <= 0) return;
    
    if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("="))
        lexpr = notation_obj_lexpr_new(argc - 1, argv + 1);
    else
        new_pitch = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
    
    lock_general_mutex((t_notation_obj *)x);
    curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
    while (curr_it) {
        if (curr_it->type == k_NOTE) {
            changed |= change_note_pitch_from_lexpr_or_llll((t_notation_obj *)x, (t_note *) curr_it, lexpr, new_pitch);
        } else if (curr_it->type == k_CHORD) {
            changed |= change_chord_pitch_from_lexpr_or_llll((t_notation_obj *)x, (t_chord *) curr_it, lexpr, new_pitch);
        }
        curr_it = lambda ? NULL : curr_it->next_selected;
    }
    unlock_general_mutex((t_notation_obj *)x);
    
    if (new_pitch)
        llll_free(new_pitch);
    if (lexpr)
        lexpr_free(lexpr);
    
    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_PITCH_FOR_SELECTION);
}


void roll_sel_change_voice(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_lexpr *lexpr = NULL;
	t_llll *new_voice = NULL;
	t_notation_item *curr_it;
	char changed = 0;
	char lambda = (s == _llllobj_sym_lambda);
	
	if (argc <= 0) return;

	if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("="))
		lexpr = notation_obj_lexpr_new(argc - 1, argv + 1);
	else
		new_voice = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
	
	lock_general_mutex((t_notation_obj *)x);
	curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
	while (curr_it) {
        t_notation_item *next_selected = curr_it->next_selected;
		if (curr_it->type == k_NOTE) {
			changed |= change_note_voice_from_lexpr_or_llll(x, (t_note *) curr_it, lexpr, new_voice, true);
		} else if (curr_it->type == k_CHORD) {
			changed |= change_chord_voice_from_lexpr_or_llll(x, (t_chord *) curr_it, lexpr, new_voice, true);
		}
		curr_it = lambda ? NULL : next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);
	
	if (changed) {
		check_correct_scheduling((t_notation_obj *)x, true);
		recompute_total_length((t_notation_obj *)x);
	}
	
	if (new_voice) 
		llll_free(new_voice);
	if (lexpr)
		lexpr_free(lexpr);

	move_preselecteditems_to_selection((t_notation_obj *)x, k_SELECTION_MODE_FORCE_SELECT, false, false);

	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_CENTS_FOR_SELECTION);
}


t_note *roll_slice_note(t_roll *x, t_note *note, double ms_pos)
{
    if (note && note->parent && ms_pos > note->parent->onset && ms_pos < note->parent->onset + note->duration) {
        create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)note->parent, k_UNDO_MODIFICATION_CHANGE);
        
        t_chord *right_ch = addchord_from_notes(x, note->parent->voiceparent->v_ob.number, ms_pos, 0, 0, NULL, NULL, false, 0);
        if (right_ch)
            create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *) right_ch, k_UNDO_MODIFICATION_DELETE);
        
        t_note *right_nt = slice_note((t_notation_obj *)x, note, ms_pos - note->parent->onset);
        insert_note((t_notation_obj *)x, right_ch, right_nt, 0);
        return right_nt;
    }
    return NULL;
}

void slice_voice_at_position(t_roll *x, t_rollvoice *voice, double position)
{
	t_chord *ch;
	t_note *nt;
	for (ch = voice->firstchord; ch; ch = ch->next) {
		t_llll *notes_to_slice = NULL;
		for (nt = ch->firstnote; nt; nt = nt->next) {
			if (ch->onset < position && ch->onset + nt->duration > position)  {
				if (!notes_to_slice)
					notes_to_slice = llll_get();
				llll_appendobj(notes_to_slice, nt, 0, WHITENULL_llll);
			}
		}

		if (notes_to_slice) {
			create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *) ch, k_UNDO_MODIFICATION_CHANGE);

			t_chord *right_ch = addchord_from_notes(x, voice->v_ob.number, position, 0, 0, NULL, NULL, false, 0);
			if (right_ch) 
				create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *) right_ch, k_UNDO_MODIFICATION_DELETE);

			t_llllelem *note_el;
			for (note_el = notes_to_slice->l_head; note_el; note_el = note_el->l_next) {
				t_note *nt = (t_note *)hatom_getobj(&note_el->l_hatom); // old note
				t_note *right_nt = slice_note((t_notation_obj *)x, nt, position - ch->onset);
				
				insert_note((t_notation_obj *)x, right_ch, right_nt, 0);
			}
		}
	}
}

void slice(t_roll *x, t_llllelem *onsets, t_llll *voices)
{
	long i;
	
	if (!onsets)
		return;

	t_llll *voices_cloned = voices ? llll_clone(voices) : NULL;
	if (voices_cloned)
		llll_develop_ranges_and_parse_negative_indices_inplace(&voices_cloned, x->r_ob.num_voices, true);
	
	t_rollvoice *voice;
	for (voice = x->firstvoice, i = 0; voice && voice->v_ob.number < x->r_ob.num_voices && i < x->r_ob.num_voices; i++, voice = voice->next) {
		if (!voices_cloned || voices_cloned->l_size == 0 || is_long_in_llll_first_level(voices_cloned, i)) {
			if (is_hatom_number(&onsets->l_hatom))
				slice_voice_at_position(x, voice, hatom_getdouble(&onsets->l_hatom));
			else if (hatom_gettype(&onsets->l_hatom) == H_LLLL) {
				t_llllelem *temp;
				for (temp = hatom_getllll(&onsets->l_hatom)->l_head; temp; temp = temp->l_next)
					if (is_hatom_number(&temp->l_hatom))
						slice_voice_at_position(x, voice, hatom_getdouble(&temp->l_hatom));
			}
		}
	}
	
	if (voices_cloned)
		llll_free(voices_cloned);
}


void roll_slice(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_llll *arguments = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);

	lock_general_mutex((t_notation_obj *)x);
	slice(x, arguments->l_head, arguments->l_head && arguments->l_head->l_next && hatom_gettype(&arguments->l_head->l_next->l_hatom) == H_LLLL ? 
		  hatom_getllll(&arguments->l_head->l_next->l_hatom) : NULL);
	unlock_general_mutex((t_notation_obj *)x);

	llll_free(arguments);
	
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_SLICE);
}



void roll_sel_change_velocity(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_lexpr *lexpr = NULL;
	t_llll *new_velocity = NULL;
	t_notation_item *curr_it;
	char changed = 0;
	char lambda = (s == _llllobj_sym_lambda);

	if (argc <= 0) return;
	
	if (atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("="))
		lexpr = notation_obj_lexpr_new(argc - 1, argv + 1);
	else
		new_velocity = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
	
	lock_general_mutex((t_notation_obj *)x);
	curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
	while (curr_it) {
        switch (curr_it->type) {
            case k_NOTE:
                changed |= change_note_velocity_from_lexpr_or_llll((t_notation_obj *)x, (t_note *) curr_it, lexpr, new_velocity);
                break;
                
            case k_PITCH_BREAKPOINT:
                changed |= change_breakpoint_velocity_from_lexpr_or_llll((t_notation_obj *)x, (t_bpt *) curr_it, lexpr, new_velocity);
                break;
                
            case k_CHORD:
                changed |= change_chord_velocity_from_lexpr_or_llll((t_notation_obj *)x, (t_chord *) curr_it, lexpr, new_velocity);
                break;
            
            default:
                break;
		}
		curr_it = lambda ? NULL : curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);

	if (new_velocity) 
		llll_free(new_velocity);
	if (lexpr)
		lexpr_free(lexpr);

	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_VELOCITY_FOR_SELECTION);
}


void roll_sel_erase_breakpoints(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_notation_item *curr_it;
	char lambda = (s == _llllobj_sym_lambda);
	char changed = 0;

	lock_general_mutex((t_notation_obj *)x);
	curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
	changed = 0;
	while (curr_it) {
		if (curr_it->type == k_NOTE) {
			t_note *nt = (t_note *) curr_it;
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
				create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)nt->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
				note_delete_breakpoints((t_notation_obj *) x, nt);
				changed = 1;
			}
		} else if (curr_it->type == k_CHORD) {
			t_chord *ch = (t_chord *) curr_it;
			t_note *nt; 
			for (nt=ch->firstnote; nt; nt = nt->next) {
				if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
					create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
					note_delete_breakpoints((t_notation_obj *) x, nt);
					changed = 1;
				}
			}
		}
		curr_it = lambda ? NULL : curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);

	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_ERASE_BREAKPOINTS_FOR_SELECTION);
}

void roll_sel_add_breakpoint(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	double rel_x_pos, y_pos, slope;
	long vel = 0; 
	char auto_vel;
	t_notation_item *curr_it;
	char lambda = (s == _llllobj_sym_lambda);

	char changed = 0;
	if (argc < 2) 
		return;
	rel_x_pos = atom_getfloat(argv);
	y_pos = atom_getfloat(argv+1);
	
	slope = (argc >= 3) ? atom_getfloat(argv+2) : 0.;
	if (slope < -1.) 
		slope = -1.; 
	if (slope > 1.) 
		slope = 1.;
	
	if (argc >= 4) {
		auto_vel = false;
		vel = atom_getlong(argv+3);
	} else 
		auto_vel = true;
	
	lock_general_mutex((t_notation_obj *)x);
	curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
	changed = 0;
	while (curr_it) {
		if (curr_it->type == k_NOTE) {
			t_note *nt = (t_note *) curr_it;
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
				create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)nt->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
				add_breakpoint((t_notation_obj *) x, nt, rel_x_pos, y_pos, slope, 0, vel, auto_vel);
				changed = 1;
			}
		} else if (curr_it->type == k_CHORD) {
			t_chord *ch = (t_chord *) curr_it;
			t_note *nt; 
			for (nt=ch->firstnote; nt; nt = nt->next) {
				if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
					create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
					add_breakpoint((t_notation_obj *) x, nt, rel_x_pos, y_pos, slope, 0, vel, auto_vel);
					changed = 1;
				}
			}
		}
		curr_it = lambda ? NULL : curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);

	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_ADD_BREAKPOINTS_TO_SELECTION);
}


void roll_sel_add_slot(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_llll *slot_as_llll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE); // We clone it: we operate destructively
	char lambda = (s == _llllobj_sym_lambda);
	char changed = 0;
	
	if (slot_as_llll) {
		t_notation_item *curr_it;
		lock_general_mutex((t_notation_obj *)x);
		curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
		while (curr_it) {
			if (curr_it->type == k_NOTE) {
				t_note *nt = (t_note *) curr_it;
				if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
					t_llll *llllcopy;	
					create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)nt->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
					llllcopy = llll_clone(slot_as_llll);
					set_slots_values_to_note_from_llll((t_notation_obj *) x, nt, slot_as_llll);
					llll_free(llllcopy);
					changed = 1;
				}
			} else if (curr_it->type == k_CHORD) {
                t_chord *ch = (t_chord *) curr_it;
                t_note *nt;
                for (nt=ch->firstnote; nt; nt = nt->next) {
                    if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
                        t_llll *llllcopy;
                        create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
                        llllcopy = llll_clone(slot_as_llll);
                        set_slots_values_to_note_from_llll((t_notation_obj *) x, nt, slot_as_llll);
                        llll_free(llllcopy);
                        changed = 1;
                    }
                }
			}
			curr_it = lambda ? NULL : curr_it->next_selected;
		}
		unlock_general_mutex((t_notation_obj *)x);
	}
	
	llll_free(slot_as_llll);

	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_ADD_SLOTS_TO_SELECTION);
}

void roll_sel_erase_slot(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	long slotnum;
	char lambda = (s == _llllobj_sym_lambda);
	
    if (argc < 1) {
        object_error((t_object *)x, "Not enough arguments!");
		return;
    }
	
// arguments are: slot#, position, new value.
	
	slotnum = (atom_gettype(argv) == A_SYM ? slotname_to_slotnum((t_notation_obj *) x, atom_getsym(argv)) : atom_getlong(argv)-1);
    if (slotnum < 0 || slotnum > CONST_MAX_SLOTS) {
        object_error((t_object *)x, "Wrong slot number!");
		return;
    }

    notationobj_sel_erase_slot((t_notation_obj *)x, slotnum, lambda);
    
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_ERASE_SLOTS_FOR_SELECTION);
}


void roll_sel_move_slot(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    char lambda = (s == _llllobj_sym_lambda);
    
    if (argc < 2) {
        object_error((t_object *)x, "Not enough arguments!");
        return;
    }
    
    // arguments are: slot#, position, new value.
    
    long from = (atom_gettype(argv) == A_SYM ? slotname_to_slotnum((t_notation_obj *) x, atom_getsym(argv)) : atom_getlong(argv)-1);
    long to = (atom_gettype(argv + 1) == A_SYM ? slotname_to_slotnum((t_notation_obj *) x, atom_getsym(argv + 1)) : atom_getlong(argv + 1)-1);
    if (from < 0 || to < 0 || from >= CONST_MAX_SLOTS || to >= CONST_MAX_SLOTS) {
        object_error((t_object *)x, "Wrong slot numbers!");
        return;
    }
    
    notationobj_sel_move_slot((t_notation_obj *)x, from, to, false, lambda);

    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_MOVE_SLOTS_FOR_SELECTION);
}


void roll_sel_copy_slot(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    char lambda = (s == _llllobj_sym_lambda);
    
    if (argc < 2) {
        object_error((t_object *)x, "Not enough arguments!");
        return;
    }
    
    // arguments are: slot#, position, new value.
    
    long from = (atom_gettype(argv) == A_SYM ? slotname_to_slotnum((t_notation_obj *) x, atom_getsym(argv)) : atom_getlong(argv)-1);
    long to = (atom_gettype(argv + 1) == A_SYM ? slotname_to_slotnum((t_notation_obj *) x, atom_getsym(argv + 1)) : atom_getlong(argv + 1)-1);
    if (from < 0 || to < 0 || from >= CONST_MAX_SLOTS || to >= CONST_MAX_SLOTS) {
        object_error((t_object *)x, "Wrong slot numbers!");
        return;
    }
    
    notationobj_sel_move_slot((t_notation_obj *)x, from, to, true, lambda);
    
    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_COPY_SLOTS_FOR_SELECTION);
}






void roll_sel_dumpselection(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    t_llll *router_ll = NULL;
    t_llll *args = llllobj_parse_llll((t_object *)x, LLLL_OBJ_UI, s, argc, argv, LLLL_PARSE_CLONE);
    llll_parseargs_and_attrs((t_object *)x, args, "l", gensym("router"), &router_ll);
	evaluate_selection(x, 0, true, router_ll);
    llll_free(args);
    llll_free(router_ll);
}

void roll_sel_sendcommand(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	if (argc > 0) {
		long command_num = atom_getlong(argv) - 1;
		if ((command_num >= 0) && (command_num < CONST_MAX_COMMANDS))
			selection_send_command(x, 0, command_num, true);
	}
}

// arguments are: slot#, position, new value (as llll).
void roll_sel_change_slot_value(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	long slotnum, position;
	t_llll *new_values_as_llll;
	char lambda = (s == _llllobj_sym_lambda);
	char changed = 0;
	
	if (argc < 3) 
		return;

	slotnum = (atom_gettype(argv) == A_SYM ? slotname_to_slotnum((t_notation_obj *) x, atom_getsym(argv)) : atom_getlong(argv)-1);
	if (slotnum < 0)
		return;

    if (atom_gettype(argv+1) == A_SYM && atom_getsym(argv+1) == _llllobj_sym_all)
        position = -1;
    else
        position = atom_getlong(argv+1);
	new_values_as_llll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc-2, argv+2, LLLL_PARSE_CLONE);
		
	lock_general_mutex((t_notation_obj *)x);
	if (new_values_as_llll) {
		t_notation_item *curr_it = lambda ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : x->r_ob.firstselecteditem;
		while (curr_it) {
			if (curr_it->type == k_NOTE) {
				t_note *nt = (t_note *) curr_it;
				if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
					create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)nt->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
					change_note_slot_value((t_notation_obj *) x, nt, slotnum, position, new_values_as_llll);
					changed = 1;
				}
			} else if (curr_it->type == k_CHORD) {
				t_chord *ch = (t_chord *) curr_it;
                t_note *nt;
                for (nt=ch->firstnote; nt; nt = nt->next) {
                    if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
                        create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
                        change_note_slot_value((t_notation_obj *) x, nt, slotnum, position, new_values_as_llll);
                        changed = 1;
                    }
                }
			}
			curr_it = lambda ? NULL : curr_it->next_selected;
		}
	}
	unlock_general_mutex((t_notation_obj *)x);
	
	llll_free(new_values_as_llll);
	
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_SLOTS_FOR_SELECTION);
}


void roll_addmarker(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_llll *args = llllobj_parse_llll((t_object *)x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
	if (args->l_size >= 2) { // position, name, role, content
		double pos_ms;
		t_symbol *role = _llllobj_sym_none;
		t_llll *content = NULL;
		
		if (hatom_gettype(&args->l_head->l_hatom) == H_SYM && hatom_getsym(&args->l_head->l_hatom) == _llllobj_sym_cursor) 
			pos_ms = (!x->r_ob.playing ? x->r_ob.play_head_start_ms : x->r_ob.play_head_ms);
        else if (hatom_gettype(&args->l_head->l_hatom) == H_SYM && hatom_getsym(&args->l_head->l_hatom) == _llllobj_sym_end)
            pos_ms = x->r_ob.length_ms_till_last_note;
		else
			pos_ms = hatom_getdouble(&args->l_head->l_hatom);

		create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
		
		if (args->l_head->l_next->l_next && hatom_gettype(&args->l_head->l_next->l_next->l_hatom) == H_SYM)
			role = hatom_getsym(&args->l_head->l_next->l_next->l_hatom);

		if (args->l_head->l_next->l_next && args->l_head->l_next->l_next->l_next && hatom_gettype(&args->l_head->l_next->l_next->l_next->l_hatom) == H_LLLL)  {
			content = llll_clone(hatom_getllll(&args->l_head->l_next->l_next->l_next->l_hatom));
		}

		t_llll *names = get_names_from_llllelem((t_notation_obj *)x, args->l_head->l_next);
		
		lock_markers_mutex((t_notation_obj *)x);
		add_marker((t_notation_obj *) x, names, pos_ms, build_timepoint(0, long2rat(0)), k_MARKER_ATTACH_TO_MS, sym_to_marker_role(role), content, 0);
		unlock_markers_mutex((t_notation_obj *)x);

		llll_free(names);
		recompute_total_length((t_notation_obj *)x);
		update_hscrollbar((t_notation_obj *)x, 0);
		
		handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_ADD_MARKER);
	}
	llll_free(args);
}

void roll_deletemarker(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_llll *args = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_RETAIN);
	if (args->l_size >= 1) {
		char res;
		create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
		lock_markers_mutex((t_notation_obj *)x);;
		res = delete_marker_by_name((t_notation_obj *) x, args);
		unlock_markers_mutex((t_notation_obj *)x);;
		if (res) {
			recompute_total_length((t_notation_obj *)x);
			update_hscrollbar((t_notation_obj *)x, 0);
			handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_DELETE_MARKER);
		} else
			remove_all_free_undo_ticks((t_notation_obj *)x, true);
	}
	llll_free(args);
}

void roll_markername(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_llll *args = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
	if (args->l_size >= 1) { // position, name
		char incremental = find_long_arg_attr_key(args, gensym("incremental"), 0, true);
		create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
		if (change_selected_markers_name((t_notation_obj *) x, args, incremental))
			handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_MARKER_NAME);
		else
			remove_all_free_undo_ticks((t_notation_obj *)x, true);
	}
	llll_free(args);
}

void roll_getmarker(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_llll *args = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE);
    char namefirst = find_long_arg_attr_key(args, gensym("namefirst"), 0, true);
	if (args->l_size - get_num_llll_in_llll_first_level(args) >= 1) {
		t_marker *marker;
		t_llll *marker_llll = NULL;

		lock_markers_mutex((t_notation_obj *)x);
		marker = markername2marker((t_notation_obj *) x, args);
		if (marker)
			marker_llll = get_single_marker_as_llll((t_notation_obj *) x, marker, namefirst);
		unlock_markers_mutex((t_notation_obj *)x);
		if (marker_llll) {
			llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, marker_llll);
			llll_free(marker_llll);
		}
	} else {
		send_marker((t_notation_obj *) x, NULL, namefirst, 6);
	}
	llll_free(args);
}

//returns true if need a redraw
char force_inscreen_ms(t_roll *x, double inscreen_ms, char send_domain_if_changed){
	if (inscreen_ms >= x->r_ob.screen_ms_end) {
		x->r_ob.screen_ms_start = x->r_ob.screen_ms_end + floor((inscreen_ms - x->r_ob.screen_ms_end) / x->r_ob.domain) *  x->r_ob.domain;
        x->r_ob.screen_ux_start = onset_to_unscaled_xposition((t_notation_obj *)x, x->r_ob.screen_ms_start);
        update_hscrollbar((t_notation_obj *)x, 2);
		if (send_domain_if_changed) 
			send_domain(x, 6, NULL);
		return true;
	} else if (inscreen_ms <= x->r_ob.screen_ms_start) {
		x->r_ob.screen_ms_start = inscreen_ms;
        x->r_ob.screen_ux_start = onset_to_unscaled_xposition((t_notation_obj *)x, x->r_ob.screen_ms_start);
		update_hscrollbar((t_notation_obj *)x, 2);
		if (send_domain_if_changed) 
			send_domain(x, 6, NULL);
		return true;
	}	
	return false;
}

//always returns true
char force_inscreenpos_ms(t_roll *x, double position, double inscreen_ms, char also_send_domain, char also_check_scheduling, char also_move_mousedown_pt){
	double temp = inscreen_ms + x->r_ob.domain * (1 - position) + CONST_TOTAL_ULENGTH_ADD * x->r_ob.zoom_y;
	double old_screen_ms_start = x->r_ob.screen_ms_start;

	if (temp > x->r_ob.length_ms) {
		if (x->r_ob.length_ms > x->r_ob.domain)
			x->r_ob.length_ms = temp;
		else
			x->r_ob.length_ms = temp - CONST_TOTAL_ULENGTH_ADD * x->r_ob.zoom_y;
	}
    x->r_ob.length_ux = x->r_ob.length_ms * CONST_X_SCALING;

	if (also_check_scheduling)
		check_correct_scheduling((t_notation_obj *)x, true);

	x->r_ob.screen_ms_start = inscreen_ms - x->r_ob.domain * position;
	if (x->r_ob.screen_ms_start < 0)
		x->r_ob.screen_ms_start = 0;
    x->r_ob.screen_ux_start = onset_to_unscaled_xposition((t_notation_obj *)x, x->r_ob.screen_ms_start);

	update_hscrollbar((t_notation_obj *)x, 2);

	if (also_move_mousedown_pt) {
		double delta_x = deltaonset_to_deltaxpixels((t_notation_obj *)x, x->r_ob.screen_ms_start - old_screen_ms_start);
		x->r_ob.j_mousedown_point.x -= delta_x;
	}
	
	if (also_send_domain) 
		send_domain(x, 6, NULL);
	return true;
}

// forces a given point to be in the screen, but exactly on the start or on the end point of the domain!
// returns 1 if point is overflown at right, -1 if it overflown at left, 0 if it was already in domain (->nothing has been done)
char force_inscreen_ms_to_boundary(t_roll *x, double inscreen_ms, char clip_to_length, char send_domain_if_changed, char also_check_scheduling, char also_move_mousedown_pt){
	if (clip_to_length) 
		inscreen_ms = CLAMP(inscreen_ms, 0, x->r_ob.length_ms);

	if (inscreen_ms > x->r_ob.screen_ms_end) {
		force_inscreenpos_ms(x, 1., inscreen_ms, send_domain_if_changed, also_check_scheduling, also_move_mousedown_pt);
		return 1;
	} else if (inscreen_ms < x->r_ob.screen_ms_start) {
		force_inscreenpos_ms(x, 0., inscreen_ms, send_domain_if_changed, also_check_scheduling, also_move_mousedown_pt);
		return -1;
	} else 
		return 0;
}

// forces a given point to be in the screen, if it overflows at right, sets that point as the left beginning (useful for playing)
// returns 1 if point is overflown at right, -1 if it overflown at left, 0 if it was already in domain (->nothing has been done)
char force_inscreen_ms_rolling(t_roll *x, double inscreen_ms, char clip_to_length, char send_domain_if_changed, char also_check_scheduling, char also_move_mousedown_pt){
	if (clip_to_length) 
		inscreen_ms = CLAMP(inscreen_ms, 0, x->r_ob.length_ms);
	
	if (inscreen_ms > x->r_ob.screen_ms_end) {
		force_inscreenpos_ms(x, 0., MIN(inscreen_ms, x->r_ob.length_ms - x->r_ob.domain), send_domain_if_changed, also_check_scheduling, also_move_mousedown_pt);
		return 1;
	} else if (inscreen_ms < x->r_ob.screen_ms_start) {
		force_inscreenpos_ms(x, 0., inscreen_ms, send_domain_if_changed, also_check_scheduling, also_move_mousedown_pt);
		return -1;
	} else 
		return 0;
}



// for inspector
void force_notation_item_inscreen(t_roll *x, t_notation_item *it, void *dummy){
	if (it->type == k_CHORD)
		force_inscreen_ms_rolling(x, ((t_chord *)it)->onset, 0, true, false, false);
	else if (it->type == k_NOTE)
		force_inscreen_ms_rolling(x, ((t_note *)it)->parent->onset, 0, true, false, false);
}


void roll_inscreen(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	if (argc >= 1 && (is_atom_number(argv) || atom_gettype(argv) == A_SYM)) {
		double inscreen = 0;
		if (atom_gettype(argv) == A_SYM) {
            if (atom_getsym(argv) == _llllobj_sym_end) {
                inscreen = x->r_ob.length_ms_till_last_note;
            } else if (atom_getsym(argv) == _llllobj_sym_cursor) {
                inscreen = x->r_ob.playing ? x->r_ob.play_head_ms : x->r_ob.play_head_start_ms;
            } else {
                t_notation_item *it;
                t_llll *args = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_RETAIN);
                lock_markers_mutex((t_notation_obj *)x);;
                it = names_to_single_notation_item((t_notation_obj *) x, args);
                if (it)
                    inscreen  = notation_item_get_onset_ms((t_notation_obj *)x, it);
                unlock_markers_mutex((t_notation_obj *)x);;
                llll_free(args);
            }
		} else
			inscreen = atom_getfloat(argv);
			
		if (force_inscreen_ms(x, inscreen, true)) {
			if (x->r_ob.notify_also_upon_messages)
				send_domain(x, 6, NULL);
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
		}
	}
}


void roll_inscreenpos(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	if (argc >= 2) {
		double screenpos = atom_getfloat(argv);
		double ms = 0;
		if (atom_gettype(argv + 1) == A_SYM) {
            if (atom_getsym(argv + 1) == _llllobj_sym_end) {
                ms = x->r_ob.length_ms_till_last_note;
            } else if (atom_getsym(argv + 1) == _llllobj_sym_cursor) {
                ms = x->r_ob.playing ? x->r_ob.play_head_ms : x->r_ob.play_head_start_ms;
            } else {
                t_notation_item *it;
                t_llll *args = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc - 1, argv + 1, LLLL_PARSE_RETAIN);
                lock_markers_mutex((t_notation_obj *)x);;
                it = names_to_single_notation_item((t_notation_obj *) x, args);
                if (it)
                    ms  = notation_item_get_onset_ms((t_notation_obj *)x, it);
                unlock_markers_mutex((t_notation_obj *)x);;
                llll_free(args);
            }
		} else if (is_atom_number(argv + 1))
			ms = atom_getfloat(argv+1);

		force_inscreenpos_ms(x, screenpos, ms, true, true, false);
		
		if (x->r_ob.notify_also_upon_messages)
			send_domain(x, 6, NULL);
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	}
}

void roll_playselection(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	// we set all the SELECTED chords as NON-played, the other ones as PLAYED
	double start_ms = -1;
	t_notation_item *selitem;
	char offline = (argc >= 1 && atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("offline"));
	t_atom av[2];

	// find selected chords and ms_boundaries
	lock_general_mutex((t_notation_obj *)x);
	for (selitem = x->r_ob.firstselecteditem; selitem; selitem = selitem->next_selected) {
		if (selitem->type == k_CHORD) {
			t_chord *chord = (t_chord *) selitem;
			if (start_ms < 0 || chord->onset < start_ms)
				start_ms = chord->onset;
		} else if (selitem->type == k_NOTE) {
			t_chord *chord = ((t_note *) selitem)->parent;
			if (start_ms < 0 || chord->onset < start_ms)
				start_ms = chord->onset;
		} else if (selitem->type == k_VOICE) {
			start_ms = 0;
			break;
		} 
	}
	unlock_general_mutex((t_notation_obj *)x);

	start_ms -= CONST_EPSILON2; // we remove an "epsilon" from the start_ms
	if (start_ms < 0.) 
		start_ms = 0.;
	
	x->r_ob.only_play_selection = true;
	
	if (offline) {
		atom_setsym(av, gensym("offline"));
		atom_setfloat(av + 1, start_ms);
	} else 
		atom_setfloat(av, start_ms);
	roll_play(x, NULL, offline ? 2 : 1, av);
}

void roll_pause(t_roll *x)
{
	x->r_ob.show_playhead = true;
	x->r_ob.play_head_start_ms = x->r_ob.play_head_ms;
	roll_stop(x, _llllobj_sym_pause, 0, NULL);
}

void roll_play(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	char offline = (argc >= 1 && atom_gettype(argv) == A_SYM && atom_getsym(argv) == gensym("offline"));
	if (offline) {
		if (bach_atomic_trylock(&x->r_ob.c_atomic_lock)) {
			object_warn((t_object *) x, "Already playing offline!");
			return;
		}
		roll_play_offline(x, s, argc - 1, argv + 1);
		bach_atomic_unlock(&x->r_ob.c_atomic_lock);
		return;
	}
	
	if (x->r_ob.playing_offline) {
		object_warn((t_object *)x, "Can't play: already playing offline");
	} else {
		schedule_delay(x, (method) roll_do_play, 0, s, argc, argv);
	}
}

void roll_play_offline(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	if (x->r_ob.playing) {
		object_warn((t_object *)x, "Can't play offline: already playing");
	} else {
		x->r_ob.playing_offline = true;
		roll_do_play(x, s, argc, argv);
		while (x->r_ob.playing_offline) {
			x->r_ob.play_step_count = x->r_ob.play_num_steps;
			roll_task(x);
		}
	}
}


void roll_do_play(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	// let's find the first chord to play
	t_notation_item *firstplayeditem = NULL; 
	double start_ms = (argc > 0) ? atom_getfloat(argv) : 0;
	double end_ms, firstplayeditem_onset = 0;
	t_chord *temp_ch = NULL;
	t_note *temp_nt = NULL;
	t_rollvoice *voice;
	long i = 0;
	
	// detecting end_ms
	if (argc > 1) {
		// end is given as argument
		end_ms = atom_getfloat(argv + 1);
		x->r_ob.play_head_fixed_end_ms = end_ms;
	} else {
		// automatic end: when the roll is over
		end_ms = x->r_ob.length_ms;
		x->r_ob.play_head_fixed_end_ms = -1;
	}
	
	// wrong time boundaries for play?
	if (end_ms > 0 && end_ms <= start_ms) 
		return;
	
	// synchronizing the playhead with out start_ms
	if (argc == 0 && x->r_ob.show_playhead)
		start_ms = x->r_ob.play_head_start_ms;
	else
		x->r_ob.play_head_start_ms = start_ms;
	
	update_playhead_cant_trespass_loop_end((t_notation_obj *)x);

	// first we send the playhead starting position
	send_playhead_position((t_notation_obj *) x, 6); 

	// This line is no more needed, since we do it constantly at the stop method:
	// set_everything_unplayed(x)
	
	// then we send partial notes, if needed
	// i.e. the chords whose onset is < start_ms but whose duration continue at start_ms
	if (x->r_ob.play_partial_notes) {
		t_llll *to_send = llll_get();
		t_llll *to_send_references = llll_get();
		char is_notewise = true;

		lock_general_mutex((t_notation_obj *)x);
		for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
			for (temp_ch = voice->firstchord; temp_ch; temp_ch = temp_ch->next){
				if (temp_ch->onset >= start_ms) {
					break;
				} else if (should_element_be_played((t_notation_obj *) x, (t_notation_item *)temp_ch)){
					for (temp_nt = temp_ch->firstnote; temp_nt && !temp_ch->played; temp_nt = temp_nt->next) {
						if (temp_ch->onset + temp_nt->duration - CONST_EPSILON1 > start_ms) {
//						if (temp_ch->onset + temp_nt->duration >= start_ms && !(temp_ch->r_it.flags & k_FLAG_PLAYED)) {
							t_llll *this_llll, *references;
							t_note *nt;
                            this_llll = chord_get_as_llll_for_sending((t_notation_obj *) x, temp_ch, x->r_ob.play_partial_notes > 1 ? k_CONSIDER_FOR_PLAYING_AS_PARTIAL_NOTE_VERBOSE : k_CONSIDER_FOR_PLAYING_AS_PARTIAL_NOTE, -1, NULL, &references, &is_notewise);
							
							// we set the partial notes as played
							temp_ch->played = true;
							for (nt = temp_ch->firstnote; nt; nt = nt->next) {
								if (temp_ch->onset + nt->duration >= start_ms) {
									nt->played = true;
									llll_appendobj(x->r_ob.notes_being_played, nt, 0, WHITENULL_llll);
								} else if (temp_ch->played)
									temp_ch->played = false;
							}
														
							llll_appendllll(to_send, this_llll, 0, WHITENULL_llll);
							llll_appendllll(to_send_references, references, 0, WHITENULL_llll);
							break;
						}
					}
				}
			}
		}
		
		llll_flatten(to_send, 1, 0);
		llll_flatten(to_send_references, 0, 0);

		unlock_general_mutex((t_notation_obj *)x);

		send_sublists_through_playout_and_free((t_notation_obj *) x, 6, to_send, to_send_references, is_notewise);
	}

	// setting the chord_play_cursor to NULL for every voice (why every voice, and not just the used ones???)
	for (i = 0; i < CONST_MAX_VOICES; i++)
		x->r_ob.chord_play_cursor[i] = NULL;
    
	x->r_ob.marker_play_cursor = NULL;
	
	lock_general_mutex((t_notation_obj *)x);
	
	x->r_ob.dont_schedule_loop_end = x->r_ob.dont_schedule_loop_start = false;

	firstplayeditem = get_next_item_to_play((t_notation_obj *)x, start_ms);
	if (firstplayeditem)
		firstplayeditem_onset = notation_item_get_onset_ms((t_notation_obj *)x, firstplayeditem);
	
	// if the first chord is beyond the end limit, we stop
	if (firstplayeditem && firstplayeditem_onset >= end_ms) 
		firstplayeditem = NULL; 
	
	if (firstplayeditem || start_ms <= end_ms) { 

		x->r_ob.scheduled_item = firstplayeditem;
		x->r_ob.scheduled_ms = firstplayeditem ? firstplayeditem_onset : end_ms;

		// if we're scheduling loop start or end, we don't want it to be rescheduled for next event
		x->r_ob.dont_schedule_loop_start = (x->r_ob.scheduled_item && x->r_ob.scheduled_item->type == k_LOOP_START) ? true : false;
		x->r_ob.dont_schedule_loop_end = (x->r_ob.scheduled_item && x->r_ob.scheduled_item->type == k_LOOP_END) ? true : false;
		
		unlock_general_mutex((t_notation_obj *)x);
		
		x->r_ob.play_head_ms = start_ms;

		if (x->r_ob.catch_playhead && force_inscreen_ms_rolling(x, x->r_ob.play_head_ms, 0, true, false, false))
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);

		x->r_ob.playing = true;
		llllobj_outlet_symbol_as_llll((t_object *)x, LLLL_OBJ_UI, 6, _llllobj_sym_play);

		if (x->r_ob.theoretical_play_step_ms <= 0){
			// step at each scheduled event
			x->r_ob.play_step_ms = (x->r_ob.scheduled_ms - start_ms);
			x->r_ob.play_num_steps = 1;
		} else {
			// fluid scheduling for redraw
			x->r_ob.play_num_steps = MAX(1, round((x->r_ob.scheduled_ms - start_ms) / x->r_ob.theoretical_play_step_ms));
			x->r_ob.play_step_ms = (x->r_ob.scheduled_ms - start_ms)/x->r_ob.play_num_steps;
		}
		x->r_ob.play_step_count = 0;

		if (!x->r_ob.playing_offline) {
			setclock_fdelay(x->r_ob.setclock->s_thing, x->r_ob.m_clock, x->r_ob.play_step_ms);
			setclock_getftime(x->r_ob.setclock->s_thing, &x->r_ob.start_play_time);
			
			if (x->r_ob.highlight_played_notes)
				invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
			else
                notationobj_redraw((t_notation_obj *) x);
		}
	} else
		unlock_general_mutex((t_notation_obj *)x);
}


void roll_stop(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	schedule_delay(x, (method) roll_do_stop, 0, s, argc, argv);
}

void roll_do_stop(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	lock_general_mutex((t_notation_obj *)x);
	x->r_ob.playing = false;
	set_everything_unplayed(x);
	x->r_ob.play_head_ms = -1;
	x->r_ob.dont_schedule_loop_end = x->r_ob.dont_schedule_loop_start = false;
	setclock_unset(x->r_ob.setclock->s_thing, x->r_ob.m_clock);
	x->r_ob.scheduled_item = NULL;
	x->r_ob.only_play_selection = false;
	x->r_ob.play_step_count = 0;
	unlock_general_mutex((t_notation_obj *)x);
	
    llllobj_outlet_symbol_as_llll((t_object *)x, LLLL_OBJ_UI, 6, s ? s : _llllobj_sym_stop);
    if (x->r_ob.highlight_played_notes)
        invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
    else
        notationobj_redraw((t_notation_obj *) x);
}



void set_everything_unplayed(t_roll *x){
	t_rollvoice *voice; t_chord *chord; t_note *note;
	for (voice = x->firstvoice; voice && (voice->v_ob.number < x->r_ob.num_voices); voice = voice->next)
		for (chord = voice->firstchord; chord; chord = chord->next) {
			chord->played = false;
			for (note = chord->firstnote; note; note = note->next) 
				note->played = false;
		}
	llll_clear(x->r_ob.notes_being_played);
}




void roll_task(t_roll *x){ 

	x->r_ob.play_head_ms += x->r_ob.play_step_ms;
	x->r_ob.play_step_count++;

	if (x->r_ob.highlight_played_notes)
		check_unplayed_notes((t_notation_obj *) x, x->r_ob.play_head_ms);
	
	if (x->r_ob.play_step_count < x->r_ob.play_num_steps) {
	
		// we haven't reached the next event: we just redraw the playline
		setclock_fdelay(x->r_ob.setclock->s_thing, x->r_ob.m_clock, x->r_ob.play_step_ms);
		if (x->r_ob.theoretical_play_step_ms > 0) {
			if (x->r_ob.catch_playhead && force_inscreen_ms_rolling(x, x->r_ob.play_head_ms, 0, true, false, false))
				invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
            notationobj_redraw((t_notation_obj *) x);
		}
		
	} else {
		
		// we have reached the next scheduled event
		lock_general_mutex((t_notation_obj *)x);

		if (x->r_ob.scheduled_item) {
			// next event is a notation item
			char scheduled_item_type = x->r_ob.scheduled_item->type;
			t_rollvoice *voice;
			t_chord *temp_ch;
			t_marker *marker;
			t_notation_item *nextitemtoplay = NULL; 
			long count = 0, i;
			double last_scheduled_ms, nextitemtoplay_onset = 0;
			long max_chord_per_scheduler_tick = MAX(x->r_ob.max_num_chord_per_scheduler_event, 1);
			t_notation_item **items_to_send = (t_notation_item **) bach_newptr(max_chord_per_scheduler_tick * sizeof(t_notation_item *));

			t_llll *to_send = llll_get();
			t_llll *to_send_references = llll_get();
			char is_notewise = true;

			// finding synchronous items
			if (scheduled_item_type == k_CHORD || scheduled_item_type == k_MARKER) {
				items_to_send[0] = x->r_ob.scheduled_item;
				if (items_to_send[0]->type == k_CHORD) {
					long voice_number = CLAMP(((t_chord *)items_to_send[0])->voiceparent->v_ob.number, 0, CONST_MAX_VOICES - 1);
					x->r_ob.chord_play_cursor[voice_number] = (t_chord *)items_to_send[0];
				} else if (items_to_send[0]->type == k_MARKER)
					x->r_ob.marker_play_cursor = (t_marker *)items_to_send[0];
				count = 1;

				if (x->r_ob.play_markers) {
					for (marker = x->r_ob.marker_play_cursor ? x->r_ob.marker_play_cursor->next : x->r_ob.firstmarker;
						 marker && count < x->r_ob.max_num_chord_per_scheduler_event; marker = marker->next) {
						if (should_element_be_played((t_notation_obj *) x, (t_notation_item *)marker)) {
							if (marker->position_ms == x->r_ob.scheduled_ms) {
								if (count < max_chord_per_scheduler_tick)
									items_to_send[count++] = (t_notation_item *)marker;
								x->r_ob.marker_play_cursor = marker;
							} else if (marker->position_ms > x->r_ob.scheduled_ms) {
								break;
							}
						}
					}
				}
				
				for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices && count < x->r_ob.max_num_chord_per_scheduler_event; voice = voice->next){
					for (temp_ch = x->r_ob.chord_play_cursor[voice->v_ob.number] ? x->r_ob.chord_play_cursor[voice->v_ob.number]->next : voice->firstchord;
						 temp_ch && count < x->r_ob.max_num_chord_per_scheduler_event; temp_ch = temp_ch->next){
						if (should_element_be_played((t_notation_obj *) x, (t_notation_item *)temp_ch)) {
							if (temp_ch->onset == x->r_ob.scheduled_ms) {
								if (count < max_chord_per_scheduler_tick)
									items_to_send[count++] = (t_notation_item *)temp_ch;
								x->r_ob.chord_play_cursor[voice->v_ob.number] = temp_ch;
							} else if (temp_ch->onset > x->r_ob.scheduled_ms) {
								break;
							}
						}
					}
				}
			}
			
			// we now have to find the next item to schedule
			if (!x->r_ob.playing_offline && scheduled_item_type == k_LOOP_END) {
				// looping: setting the chord_play_cursor to NULL for every voice
				for (i = 0; i < x->r_ob.num_voices; i++)
					x->r_ob.chord_play_cursor[i] = NULL;
				x->r_ob.marker_play_cursor = NULL;
				
				// we reset the start play time, and we set the starting playhead position to the loop start position 
				setclock_getftime(x->r_ob.setclock->s_thing, &x->r_ob.start_play_time);
				x->r_ob.play_head_start_ms = x->r_ob.loop_region.start.position_ms; 

				last_scheduled_ms = x->r_ob.loop_region.start.position_ms;
				x->r_ob.play_head_ms = last_scheduled_ms;
				x->r_ob.dont_schedule_loop_start = false;
			} else {
				last_scheduled_ms = x->r_ob.scheduled_ms;
				if (scheduled_item_type == k_LOOP_START)
					x->r_ob.dont_schedule_loop_end = false;
			}
			
			nextitemtoplay = get_next_item_to_play((t_notation_obj *)x, last_scheduled_ms);
			if (nextitemtoplay)
				nextitemtoplay_onset = notation_item_get_onset_ms((t_notation_obj *)x, nextitemtoplay);

			if (x->r_ob.play_head_fixed_end_ms > 0 && nextitemtoplay && nextitemtoplay_onset >= x->r_ob.play_head_fixed_end_ms)
				nextitemtoplay = NULL;

//			dev_post("Next item to play has onset: %.2f ms", nextitemtoplay_onset);
			
			// loop start must be scheduled once: if we schedule loop start, then we schedule a chord falling ON the loop start, we don't want next item to be the loop start again
			// we check if we can resume scheduling the loop start or end
			if (x->r_ob.dont_schedule_loop_start && x->r_ob.loop_region.start.position_ms != nextitemtoplay_onset)
				x->r_ob.dont_schedule_loop_start = false;
			if (x->r_ob.dont_schedule_loop_start && x->r_ob.loop_region.start.position_ms != nextitemtoplay_onset)
				x->r_ob.dont_schedule_loop_end = false;

			// we schedule the next event
			x->r_ob.scheduled_ms = nextitemtoplay ? nextitemtoplay_onset : (x->r_ob.play_head_fixed_end_ms > 0 ? x->r_ob.play_head_fixed_end_ms : x->r_ob.length_ms);
			if (x->r_ob.theoretical_play_step_ms <= 0){
				// just one step per scheduled event
				x->r_ob.play_num_steps = 1;
				x->r_ob.play_step_ms = (x->r_ob.scheduled_ms - last_scheduled_ms);
			} else {
				// fluid steps for redraw
				x->r_ob.play_num_steps = MAX(1, round((x->r_ob.scheduled_ms - last_scheduled_ms) / x->r_ob.theoretical_play_step_ms));
				x->r_ob.play_step_ms = (x->r_ob.scheduled_ms - last_scheduled_ms)/x->r_ob.play_num_steps;
			}
			
			if (nextitemtoplay) { // we check if we should or should not prevent loops from being scheduled
				if (nextitemtoplay->type == k_LOOP_START)
					x->r_ob.dont_schedule_loop_start = true;
				else if (nextitemtoplay->type == k_LOOP_END)
					x->r_ob.dont_schedule_loop_end = true;
			}

			x->r_ob.play_step_count = 0;
			x->r_ob.scheduled_item = nextitemtoplay; // this has to be within the mutex!

			// gathering chord values as llll
			for (i = 0; i < count; i++){
				t_llll *references = NULL, *this_llll = NULL;
				t_note *temp_nt;
				if (items_to_send[i]->type == k_CHORD) {
					((t_chord *)items_to_send[i])->played = true;
					for (temp_nt = ((t_chord *)items_to_send[i])->firstnote; temp_nt; temp_nt = temp_nt->next) {
						temp_nt->played = true;
						llll_appendobj(x->r_ob.notes_being_played, temp_nt, 0, WHITENULL_llll);
					}
					this_llll = chord_get_as_llll_for_sending((t_notation_obj *) x, (t_chord *)items_to_send[i], k_CONSIDER_FOR_PLAYING, -1, NULL, &references, &is_notewise);
				} else if (items_to_send[i]->type == k_MARKER) {
					t_llll *temp = get_single_marker_as_llll((t_notation_obj *) x, (t_marker *)items_to_send[i], true);
					this_llll = llll_get();
					references = llll_get();
					llll_appendobj(this_llll, temp, 0, WHITENULL_llll);
					llll_appendobj(references, items_to_send[i], 0, WHITENULL_llll);
				}
				if (this_llll && references) {
					llll_appendllll(to_send, this_llll, 0, WHITENULL_llll);
					llll_appendllll(to_send_references, references, 0, WHITENULL_llll);
				}
			}

			llll_flatten(to_send, 1, 0);
			llll_flatten(to_send_references, 0, 0);
			
			unlock_general_mutex((t_notation_obj *)x);

			if (!x->r_ob.playing_offline) 
				setclock_fdelay(x->r_ob.setclock->s_thing, x->r_ob.m_clock, x->r_ob.play_step_ms);
			
			x->r_ob.play_head_ms = last_scheduled_ms;

			if (!x->r_ob.playing_offline)
				if (x->r_ob.catch_playhead && force_inscreen_ms_rolling(x, x->r_ob.play_head_ms, 0, true, false, false))
					invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
			
			// outputting chord values
			if (count > 0)
				send_sublists_through_playout_and_free((t_notation_obj *) x, 6, to_send, to_send_references, is_notewise);
			else if (scheduled_item_type == k_LOOP_START || scheduled_item_type == k_LOOP_END) {
				llllobj_outlet_symbol_couple_as_llll((t_object *)x, LLLL_OBJ_UI, 6, _llllobj_sym_loop, scheduled_item_type == k_LOOP_START ? _llllobj_sym_start : _llllobj_sym_end);
				llll_free(to_send);
				llll_free(to_send_references);
			}
			
			if (!x->r_ob.playing_offline) {
				if (x->r_ob.highlight_played_notes)
					invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
				else
                    notationobj_redraw((t_notation_obj *) x);
			}

			bach_freeptr(items_to_send);

		} else {
			// next event is the end of the roll
			char need_repaint = (x->r_ob.playing_offline == 0); 
			t_llll *end_llll = llll_get();
			
			x->r_ob.playing = x->r_ob.playing_offline = false;
			set_everything_unplayed(x);
			x->r_ob.play_head_ms = -1;
			x->r_ob.scheduled_item = NULL;
			x->r_ob.only_play_selection = false;
			x->r_ob.play_step_count = 0;
			unlock_general_mutex((t_notation_obj *)x);
			
			// send "end" message 
			llll_appendsym(end_llll, _llllobj_sym_end, 0, WHITENULL_llll);
			llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, end_llll);
			llll_free(end_llll);
			
			if (need_repaint)
				invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
		}
		
	}
}





void roll_clock(t_roll *x, t_symbol *s) 
{
	void *old = x->r_ob.setclock->s_thing; 
	t_object *c = 0; 
	
	// the line below can be restated as: 
	// if s is the empty symbol 
	// or s->s_thing is zero 
	// or s->s_thing is non-zero and a setclock object  
	if (s && (s == gensym("") || ((c = (t_object *) s->s_thing) && zgetfn(c, gensym("unset"))))) { 
		if (old) 
            roll_stop(x, NULL, 0, NULL); 
		x->r_ob.setclock = s; 
		/*		if (x->m_running) 
		 setclock_delay(c, x->r_clock, 0L); */
	} 
}


	
void roll_clearselection(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	if (argc == 0) {
		lock_general_mutex((t_notation_obj *)x);	
		clear_preselection((t_notation_obj *) x);
		clear_selection((t_notation_obj *) x);
		unlock_general_mutex((t_notation_obj *)x);	
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	} else if (argc == 1 && atom_gettype(argv) == A_LONG) {
		lock_general_mutex((t_notation_obj *)x);	
		clear_preselection((t_notation_obj *) x);
		clear_voice_selection((t_notation_obj *) x, atom_getlong(argv) - 1);
		unlock_general_mutex((t_notation_obj *)x);	
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	}
	handle_change_selection((t_notation_obj *)x);
}


long get_global_num_notes(t_roll *x){
	t_rollvoice *voice = x->firstvoice;
	long numnotes = 0;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		numnotes += get_global_num_notes_voice(voice);
		voice = voice->next;
	}
	return numnotes;
}

long get_global_num_notes_voice(t_rollvoice *voice){
	t_chord *temp_ch = voice->firstchord;
	long numnotes = 0;
	while (temp_ch) {
		numnotes += temp_ch->num_notes;
		temp_ch = temp_ch->next;
	}
	return numnotes;
}


long get_num_chords(t_roll *x){
	t_rollvoice *voice;
	long numch = 0;
	lock_general_mutex((t_notation_obj *)x);	
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		numch += voice->num_chords;
		voice = voice->next;
	}
	unlock_general_mutex((t_notation_obj *)x);	
	return numch;
}




 /// PATTRSTORAGE STUFF
#ifdef BACH_PATTR_SUPPORT
t_max_err roll_getvalueof(t_roll *x, long *ac, t_atom **av)
{
    if (ac && av) {
        // allocate enough memory for your data
        t_llll *ll = get_roll_values_as_llll(x, k_CONSIDER_FOR_SAVING, (e_header_elems) (k_HEADER_BODY | k_HEADER_SLOTINFO | k_HEADER_VOICENAMES | k_HEADER_MARKERS | k_HEADER_GROUPS  | k_HEADER_ARTICULATIONINFO | k_HEADER_NOTEHEADINFO | k_HEADER_NUMPARTS), true, false);
        *ac = llll_deparse(ll, av, 0, /* LLLL_D_FLOAT64 | */ LLLL_D_QUOTE);
        llll_free(ll);
    }
    return MAX_ERR_NONE;
}

t_max_err roll_setvalueof(t_roll *x, long ac, t_atom *av)
{
    if (ac && av) {
        t_llll *ll = llll_parse(ac, av);
        create_whole_roll_undo_tick(x);
        set_roll_from_llll(x, ll, true);
        handle_rebuild_done((t_notation_obj *) x);
        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_ROLL);
        llll_free(ll);
    }
    return MAX_ERR_NONE;
}
#endif




int T_EXPORT main(void){
	t_class *c;
	
	common_symbols_init();
	llllobj_common_symbols_init();

	bach_inspector_ui_classinit();

	if (llllobj_check_version(BACH_LLLL_VERSION) || llllobj_test()) {
		error("bach: bad installation");
		return 1;
	}
	
	CLASS_NEW_CHECK_SIZE(c, "bach.roll", 
							(method)roll_new,
							(method)roll_free,
							(short)sizeof(t_roll),
							(method)NULL,
							A_GIMME,
							0L);
	
	c->c_flags |= CLASS_FLAG_NEWDICTIONARY;

	jbox_initclass(c, JBOX_TEXTFIELD);
	
	class_addmethod(c, (method) roll_paint, "paint", A_CANT, 0);
    
#ifdef BACH_PATTR_SUPPORT
    class_addmethod(c, (method) roll_getvalueof, "getvalueof", A_CANT, 0);
    class_addmethod(c, (method) roll_setvalueof, "setvalueof", A_CANT, 0);
#endif
    
    // @method subroll @digest Output a portion of <o>bach.roll</o>
	// @description The <m>subroll</m> message outputs the gathered syntax information of a portion of the <o>bach.roll</o>,
	// namely it outputs only certain voices and within a certain time interval.
	// The syntax for the <m>subroll</m> message is:
	// <b>subroll <m>VOICES</m> <m>TIME_LAPSE</m> <m>optional:SELECTIVE_OPTIONS</m></b>,
	// <m>VOICES</m> is an llll of the kind <b>(<m>voice_number1</m> <m>voice_number2</m>...)</b>
	// containing the number of the voices to be output; leave <b>nil</b> or <b>()</b> if you want to output all voices. <br />
	// <m>TIME_LAPSE</m> is an llll of the kind <b>(<m>start_ms</m> <m>end_ms</m>)</b> containing the time lapse that
	// has to be output. Leave such list as <b>nil</b> or <b>()</b> if you want this lapse to be all the length of the <o>bach.roll</o>.
	// Otherwise <m>start_ms</m> is the beginning of the portion of <o>bach.roll</o> to be output (in milliseconds), and 
	// <m>end_ms</m> is the end of the portion of <o>bach.roll</o> to be output (in milliseconds); leave any negative value
	// for <m>end_ms</m> if you want the portion of <o>bach.roll</o> to be output to go till the end of the notation object. <br />
	// The third llll, <m>optional:SELECTIVE_OPTIONS</m>, is optional, and if given might contain a symbol or list of symbols
	// which handle what part of the header should be dumped. By default all header is output. Options for these symbols are exactly
	// the same as for the <m>dump</m> message (see its documentation to know more). For instance <b>subroll (4 5) (1000 3000) (clefs markers body)</b>
	// output voices 4 and 5 in the portion of the <o>bach.roll</o> going from 1000ms to 3000ms, and outputs in addition to the musical content (the body)
	// the information about clefs and the markers. 
	// Leave <b>(body)</b> as third parameter if you only want to dump the music content, and no header information
	// @marg 0 @name voices @optional 0 @type llll
	// @marg 1 @name time_lapse @optional 0 @type llll
	// @marg 2 @name selective_options @optional 1 @type llll
    // @example subroll (1 2 4) (1000 3000) @caption extract voices 1, 2 and 4 from 1s to 3s
    // @example subroll () (1000 3000) @caption extract all voices from 1s to 3s
    // @example subroll (1 2) () @caption extract voices 1 and 2 for the whole duration
    // @example subroll (1 3) (2000 -1) @caption extract voices 1 and 3 from 2s to the end
    // @example subroll onset (1 3) (2000 -1) @caption only extract notes whose onset is after 2s (no partial notes)
    // @example subroll (4 5) (1000 3000) (body) @caption only dump the body
    // @example subroll (4 5) (1000 3000) (clefs markers body) @caption only dump clefs, markers and body
    // @seealso dump
	class_addmethod(c, (method) roll_subroll, "subroll", A_GIMME, 0);


	// @method merge @digest Merge notes or chords
	// @description You can use the <m>merge</m> message to perform a merging of chords too near (with respect to the time), 
	// and/or of notes too near (with respect to the pitch). The two tasks are separate tasks: the first implies that chords 
	// lying within a given time threshold will be merged to a single chord; the second implies that notes whose pitches differ 
	// by less then a pitch threshold will be merged to a single note. <br />
	// Two arguments are needed for the "merge" command: the time threshold (in milliseconds) and the pitch threshold (in cents).
	// If any of the two threshold is set as negative, this means that there will be no corresponding merging: leave the time threshold as negative
	// if you only want to merge in the pitch direction, leave the pitch threshold as negative if you only want to merge chords in the time coordinate. <br />
	// Two additional optional arguments can be given specifying, respectively: the time merging policy (-1 = align to the leftmost chord, 
	// 0 = align to the average of chords, 1 = align to the rightmost chord), the pitch/velocity merging policy (-1 = set pitch/velocity to the bottommost 
	// pitch/velocity, 0 = set pitch/velocity to the average of pitches/velocities, 1 = set pitch/velocity to the topmost pitch/velocity). 
	// The merging policies are optional parameters: if you don't specify them, by default they are 0 (= merge to the average). <br />
	// If an optional "selection" symbol is put in front of the time threshold argument, the merging is performend only on the current selection,
	// otherwise it is performed on the whole score. <br />
    // Merging also applies to markers, which actually entails a deletion of markers nearer than <m>threshold_ms</m> to some other marker.
    // Time position is averaged with <m>time_merging_policy</m> also. If you don't want markers to be thinned, consider selecting all chords only
    // and then using <m>merge selection</m>.
	// @marg 0 @name selection @optional 1 @type sym
	// @marg 1 @name threshold_ms @optional 0 @type float
	// @marg 2 @name threshold_cents @optional 0 @type float
	// @marg 3 @name time_merging_policy @optional 1 @type int
	// @marg 4 @name pitch_vel_merging_policy @optional 1 @type int
    // @example merge 200 10 @caption merge chords and markers <= 200ms and notes <= 10cents
    // @example merge -1 10 @caption just merge notes <= 10cents
    // @example merge 200 -1 @caption just merge chords and markers <= 200ms
    // @example merge selection 200 10 @caption only merge elements if selected
    // @example sel chords, merge selection 200 -1 @caption just merge chords <= 200ms
    // @example merge 200 10 -1 @caption align result to the leftmost merged chord
    // @example merge 200 10 1 -1 @caption align result to the rightmost merged chord and bottommost pitch
    // @seealso explodechords
	class_addmethod(c, (method) roll_merge, "merge", A_GIMME, 0);


	// @method inscreen @digest Change scrollbar position to display a temporal point
	// @description The message <m>inscreen</m> followed by a temporal position specified in milliseconds forces such
	// position to be displayed inside the domain, by changing the scrollbar position.
    // The millisecond position can be replaced by the element name or by one of the following symbols: "cursor", "end".
	// @marg 0 @name ms_or_name @optional 0 @type float/symbol
    // @example inscreen 5000 @caption display position at 5s in domain
    // @example inscreen john @caption display element named 'john' in domain
    // @example inscreen cursor @caption display playhead the domain
    // @example inscreen end @caption display end of roll the domain
    // @seealso inscreenpos
	class_addmethod(c, (method) roll_inscreen, "inscreen", A_GIMME, 0);


	// @method inscreenpos @digest Change scrollbar position to precisely display a temporal point 
	// @description The message <m>inscreenpos</m> sets a given position in milliseconds in a given point inside the notation object domain. 
	// It has to be followed by two argument: a value between 0 and 1, specifying the relative location inside the screen in which the temporal
	// point should be displayed (0 represents the screen beginning, 1 the screen end), and the position in milliseconds.
    // The millisecond position can be replaced by the element name or by one of the following symbols: "cursor", "end".
	// @marg 0 @name domain_relative_location @optional 0 @type float
    // @marg 0 @name ms_or_name @optional 0 @type float/symbol
    // @example inscreenpos 0.5 5000 @caption display 5s position at the exact domain center
    // @example inscreenpos 0.9 john @caption display element named 'john' at 90% of the domain
    // @example inscreenpos 0.9 cursor @caption display playhead at 90% of the domain
    // @example inscreenpos 0.9 end @caption display end of roll at 90% of the domain
    // @seealso inscreen
	class_addmethod(c, (method) roll_inscreenpos, "inscreenpos", A_GIMME, 0);


	// @method clock @digest Select a clock source
	// @description The word <m>clock</m>, followed by the name of an existing <m>setclock</m> objects, sets the <o>bach.roll</o> 
	// object to be controlled by that <m>setclock</m> object rather than by Max's internal millisecond clock. 
	// The word <m>clock</m> by itself sets the <o>bach.roll</o> object back to using Max's regular millisecond clock.
    // @marg 0 @name setclock_name @optional 1 @type symbol
    // @example clock ticker @caption link to 'ticker' setclock
    // @example clock @caption link to default clock
	class_addmethod(c, (method) roll_clock,	"clock", A_DEFSYM, 0);


	// @method delete @digest Delete current selected items
	// @description @copy BACH_DOC_MESSAGE_DELETE
    // <br />
    // When deleting notes from a chord, use the "@transferslots" specification to define a set of slots to be transfered to a neighbour note in the same chord,
    // whenever possible. Use also "all" to transfer all slots, and "auto" to transfer a standard set of slots (dynamics, lyrics, articulations, annotations);
    // use "dynamics", "lyrics", "noteheads", "articulations", "annotation" instead of slot numbers.
    // Use the additional "@empty" message argument in order to tell with a 0/1 integer whether empty slots should also be transfered (by default: 0 = false).
    // @mattr transferslots @type llll @default null @digest If non-null, when deleting notes from a chord, these slots will be transfered to another chord note whenever possible
    // @mattr empty @type int @default 0 @digest Also transfer empty slots
    // @example delete @caption delete current selection
    // @example delete @transferslots 20 21 @caption delete current selection, and transfer content of slot 20 and 21 to another chord note, if possible
    // @example delete @transferslots all @caption delete current selection, and transfer all slots
    // @example delete @transferslots all @empty 1 @caption delete current selection, and transfer all slots, even the empty ones
    // @seealso rippledelete, eraseslot
	class_addmethod(c, (method) roll_sel_delete, "delete", A_GIMME, 0);

    
    // @method rippledelete @digest Ripple-delete current selected items
    // @description The message <m>rippledelete</m> deletes all the currently selected items, and shifts all the following one
    // backwards in order to fill the gap. <br />
    // When deleting notes from a chord, use the "@transferslots" specification to define a set of slots to be transfered to a neighbour note in the same chord,
    // whenever possible. Use also "all" to transfer all slots, and "auto" to transfer a standard set of slots (dynamics, lyrics, articulations, annotations);
    // use "dynamics", "lyrics", "noteheads", "articulations", "annotation" instead of slot numbers.
    // Use the additional "@empty" message argument in order to tell with a 0/1 integer whether empty slots should also be transfered (by default: 0 = false).
    // @mattr transferslots @type llll @default null @digest If non-null, when deleting notes from a chord, these slots will be transfered to another chord note whenever possible
    // @mattr empty @type int @default 0 @digest Also transfer empty slots
    // @example rippledelete @caption ripple-delete current selection
    // @example rippledelete @transferslots 20 21 @caption ripple-delete current selection, and transfer content of slot 20 and 21 to another chord note, if possible
    // @example rippledelete @transferslots all @caption ripple-delete current selection, and transfer all slots
    // @example rippledelete @transferslots all @empty 1 @caption ripple-delete current selection, and transfer all slots, even the empty ones
    // @seealso rippledelete, eraseslot
    class_addmethod(c, (method) roll_sel_ripple_delete, "rippledelete", A_GIMME, 0);


	// @method sel @digest Select items
	// @description The word <m>sel</m> add some notation items to the current selection. In the basic behavior,
	// the word must be followed by four elements, specifying: <br />
	// - the starting temporal point of the selection, in milliseconds (leave <b>nil</b> or <b>()</b> if you want to select from the beginning); <br />
	// - the ending temporal point of the selection, in milliseconds (leave <b>nil</b> or <b>()</b> if you want to select until the end); <br />
	// - the minimum pitch of the selection, in cents (leave <b>nil</b> or <b>()</b> if you don't want to put a lower bound on pitches); <br />
	// - the maximum pitch of the selection, in cents (leave <b>nil</b> or <b>()</b> if you don't want to put an upper bound on pitches). <br />
	// Other selection modes are possible: <br />
	// - If the word <m>sel</m> is followed by the symbol <b>all</b>, all notes, chords and markers are selected. <br />
    // - If the word <m>sel</m> is followed by a category plural symbol, all the corresponding elements are selected.
    // Category plural symbols are: <b>markers</b>, <b>notes</b>, <b>chords</b>, <b>breakpoints</b>, <b>tails</b>. <br />
	// - If the word <m>sel</m> is followed by the symbol <b>markers</b>, all markers are selected. <br />
	// - If the word <m>sel</m> is followed by the symbol <b>chord</b> followed by one or two integers (representing an address), a certain chord is selected.
	// The full syntax for the integers is: <m>voice_number</m> <m>chord_index</m>. If just an element is given, the voice number is considered
	// to be by default 1. The chord index is the index of chords, sorted by onset. 
	// For instance, <b>sel chord 2</b> selects the second chord (of first voice), while <b>sel chord 3 2</b> does the same with the third voice. 
	// Negative positions are also allowed, counting backwards. Multiple chords can be selected at once, provided that instead of a list integers one gives
	// a sequence of wrapped lists of integers, for instance <b>sel chord (1 1) (1 2) (1 3) (2 1) (3 -1)</b>.<br />
	// - If the word <m>sel</m> is followed by the symbol <b>note</b> followed by one, two or three integers (representing an address), a certain note is selected.
	// The full syntax for the integers is: <m>voice_number</m> <m>chord_index</m> <m>note_index</m>. If less elements are given, the first ones are considered
	// to be by default 1's. The chord index is the index of chords, sorted by onset; the note index is taken from the lowest to the highest.
	// For instance, <b>sel note 2 3</b> selects the third note of second chord (of first voice), while <b>sel note 4 2 3</b> does the same with the fourth voice. 
	// Negative positions are also allowed, counting backwards. Multiple notes can be selected at once, provided that instead of a list integers one gives
	// a sequence of wrapped lists of integers, for instance <b>sel note (5 2 4) (1 1 -1)</b>.<br />
    // - If the word <m>sel</m> is followed by the symbols <b>note if</b>, <b>rest if</b>, <b>marker if</b>, <b>breakpoint if</b>
    // or <b>tail if</b> followed by a
    // condition to be verified, a conditional selection on notes,  markers or pitch breakpoints (respectively) is performed.
	// and notes/markers/pitch breakpoints matching such condition are selected.
    // The condition must be an expr-like expression returning 1 if notes have to be selected, 0 otherwise.
	// You can use symbolic variables inside such expressions. <br />
	// @copy BACH_DOC_SYMBOLIC_VARIABLES
	// For instance <b>sel note if velocity == 100</b> selects all notes whose
	// velocity is exactly equal to 100, while <b>round(cents / 100.) % 12 == 0</b> select all the C's.<br />
	// - If the word <m>sel</m> is followed by any other symbol or sequence of symbols, these are interpreted as names, and the notation items
	// matching all these names (or a single name, if just one symbol is entered) are selected. <br />
	// @marg 0 @name arguments @optional 0 @type llll
    // @example sel all @caption select everything
    // @example sel chords @caption select all chords
    // @example sel markers @caption select all markers
    // @example sel breakpoints @caption select all breakpoints
    // @example sel tails @caption select all tails
    // @example sel chord 3 @caption select 3rd chord (of 1st voice)
    // @example sel chord 2 4 @caption select 4th chord of 2nd voice
    // @example sel chord -1 -1 @caption select last chord of last voice
    // @example sel note 3 4 -1 @caption select last note of 4th chord of 3rd voice
    // @example sel marker 5 @caption select 5th marker
    // @example sel marker -2 @caption select one-but-last marker
    // @example sel marker (1) (-2) (5) @caption select multiple markers
    // @example sel chords (1 3) (2 2) (-2 5) @caption select multiple chord
    // @example sel notes (1 3 2) (1 3 3) (2 4 5) @caption select multiple notes
    // @example sel John @caption select all items named 'John'
    // @example sel John Lennon @caption select all items named both 'John' and 'Lennon'
    // @example sel 1000 3000 6000 7200 @caption select notes between 1s and 3s, and between 6000 and 7200cents
    // @example sel 1000 3000 6000 7200 2 @caption same, for second voice only
    // @example sel 1000 3000 6000 7200 (1 3 4) @caption same, for voices 1, 3 and 4
    // @example sel 1000 3000 () () @caption select notes between 1s and 3s
    // @example sel () () 4800 6000 @caption select notes below middle C
    // @example sel () () () () -1 @caption select every note in last voice
    // @example sel note if cents == 6000 @caption select all middle C's
    // @example sel note if (cents % 1200) == 0 @caption select all C's
    // @example sel note if (cents < 6000) && (duration < 1000) @caption select all notes <1s below middle C
    // @example sel marker if onset > 5000 @caption select all markers with onset >5s
    // @example sel breakpoint if (cents > 7200) && (velocity > 100) @caption select all pitch breakpoints >7200cents with velocity > 100
    // @example sel tail if cents > 6000 @caption select all tails above middle C
    // @seealso unsel, select, subsel, clearselection
	class_addmethod(c, (method) roll_select, "sel", A_GIMME, 0);


    // @method goto @digest Move selection
    // @description The <m>goto</m> message moves the selection to a new item, i.e. clears the selection and select the new item.
    // The first argument is the movement type: <br />
    // <b>prev</b>: select previous notation item, if any; <br />
    // <b>next</b>: select next notation item, if any. <br />
    // @example sel next @caption select next notation item
    // @example sel prev @caption select previous notation item
    // @seealso sel, select, unsel
    class_addmethod(c, (method) roll_anything, "goto", A_GIMME, 0);

    
	// @method unsel @digest Deselect items
	// @description The message <m>unsel</m> works in the opposite way of the message <m>sel</m>, i.e. deselect
	// notation items (removes them from the current selection). Refer to the message <m>sel</m> to know more about the
	// syntax, since it is exactly the same.
	// @marg 0 @name arguments @optional 0 @type llll
    // @seealso sel, select, subsel, clearselection
	class_addmethod(c, (method) roll_select, "unsel", A_GIMME, 0);


	// @method select @digest Involutively select items
	// @description The message <m>select</m> works as <m>sel</m> but in an involutive manner: i.e. it selects non-selected items
	// and deselect already selected items. This is essentially what happens when you draw a 
	// selection rectangle on the notation object, and then draw another overlapping one keeping the Shift key pressed.
	// Refer to the message <m>sel</m> to know more about the syntax, since it is exactly the same.
	// The only exception is for the <m>select all</m> message, which does not work involutively and always select all the notation items.
	// @marg 0 @name arguments @optional 0 @type llll
    // @seealso sel, unsel, subsel, clearselection
	class_addmethod(c, (method) roll_select, "select", A_GIMME, 0);

    
    // @method subsel @digest Restrict selection
    // @description The message <m>subsel</m> works as the <m>sel</m> message, but restricting the existing selection.
    // More specifically, elements are only kept as selected if they verify the specified property.
    // In other words, the <m>subsel</m> message create an intersection between the specified selection and the existing one.
    // Refer to the message <m>sel</m> to know more about the syntax, since it is exactly the same.
    // @marg 0 @name arguments @optional 0 @type llll
    // @seealso sel, unsel, select, clearselection
    class_addmethod(c, (method) roll_select, "subsel", A_GIMME, 0);
    
    

	// @method name @digest Assign name(s) to selected items
	// @description @copy BACH_DOC_MESSAGE_NAME
	// @marg 0 @name names @optional 0 @type llll
    // @mattr incremental @type int @default 0 @digest If non-zero, assigns incremental numbering to selected markers in addition to names
    // @mattr progeny @type int @default 0 @digest If non-zero, assigns names to both selected chords and all their notes
    // @example name George @caption name selected elements as 'George'
    // @example name (George Martin) (George Harrison) @caption assign complex llll name
    // @example name George @incremental 1 @caption also add unique incremental numbers to selected markers
    // @example name George @progeny 1 @caption if chords are selected also assign name to their notes
    // @seealso nameappend, clearnames, sel, unsel, select
	class_addmethod(c, (method) roll_name, "name", A_GIMME, 0);

    
	// @method nameappend @digest Append name(s) to selected items
	// @description Works like the <m>name</m> message, but appends the given names
	// to the already existing ones (see <m>name</m>).
	// @marg 0 @name names @optional 0 @type llll
    // @example name Ringo @caption append 'Ringo' to selected element's names
    // @example name (Ringo Starr) ((Low High Snare) (Sizzle Crash)) @caption append complex llll name
    // @seealso name, clearnames
	class_addmethod(c, (method) roll_nameappend, "nameappend", A_GIMME, 0);
	
	
	// @method clearnames @digest Clear all names in the score
	// @description Clears the names of all notation items in the score (this includes voice names and marker names).
	// If you want to only clear names for some category, add the category name(s) after the <m>clearnames</m> symbol. Names must be among the following ones:
    // "chords", "voices", "notes", "markers".
	// @marg 0 @name categories @optional 1 @type llll
    // @example clearnames @caption delete all names
    // @example clearnames marker @caption only delete marker names
    // @example clearnames chord note @caption only delete chord and note names
    // @seealso name, nameappend
	class_addmethod(c, (method) roll_clearnames, "clearnames", A_GIMME, 0);
	
    
    
    // @method slottoname @digest Assign name(s) to selected items from slot content
    // @description A <m>slottoname</m> message followed by an integer <m>N</m> overwrites the names of selected
    // notes and rests with the content of their slot <m>N</m>. Slot <m>N</m> is expected to be a text or llll slot.
    // N.B.: While setting notes' names, the message also clears the corresponding chords' names.
    // @marg 0 @name slot_number_or_name @optional 0 @type int/symbol
    // @seealso name, nameappend, clearnames, sel, unsel, select
    class_addmethod(c, (method) roll_slottoname, "slottoname", A_GIMME, 0);
    
    
    // @method nametoslot @digest Copy name(s) to slot for selected items
    // @description A <m>nametoslot</m> message followed by an integer <m>N</m> copies the names of the selected
    // notes inside their slot <m>N</m>. <br />
    // A second argument sets the policy in case the chord containing the note also has a name. This argument
    // must be one of the following symbols: <br />
    // - "ignore" (default): chord name is ignored; <br />
    // - "replace": chord name is always used instead of note names; <br />
    // - "replacenull": chord name is used only for notes having no name (i.e. <b>null</b> name); <br />
    // - "prepend": chord name is prepended to existing note names; <br />
    // - "append": chord name is appended to existing note names; <br />
    // - "merge" or "mergeprepend" (default): only chord names which don't already belong to the note are prepended to existing note names; <br />
    // - "mergeappend": only chord names which don't already belong to note are appendend to existing note names.
    // @marg 0 @name slot_number_or_name @optional 0 @type int/symbol
    // @marg 1 @name policy @optional 1 @type symbol
    // @seealso name, nameappend, clearnames, sel, unsel, select
    class_addmethod(c, (method) roll_nametoslot, "nametoslot", A_GIMME, 0);
    
    
    // @method role @digest Assign role to selected markers
    // @description Changes or assigns a role to all selected markers. The role has to be specified by the argument, and should be a symbol
    // among the following ones: "none", "tempo", "timesig" "barline", "division", "subdivision". If the role also requires a value or content
    // (which might be the case for
    // tempi and time signatures), this latter can be specified via a further argument as a wrapped llll.
    // @marg 0 @name role @optional 0 @type symbol
    // @marg 1 @name content @optional 1 @type llll
    // @example role barline @caption assign 'barline' role to selected markers
    // @example role timesig (3 4) @caption assign a 3/4 'time signature' role
    // @example role none @caption reset roles to none
    // @seealso name, sel, unsel, select
    class_addmethod(c, (method) roll_role, "role", A_GIMME, 0);

    
    // @method group @digest Group selected chords
    // @description A <m>group</m> message groups all selected items.
    // @seealso ungroup
    class_addmethod(c, (method) roll_group, "group", A_GIMME, 0);

    
    // @method ungroup @digest Ungroup selected items
    // @description A <m>ungroup</m> message breaks all the groups of selected chords.
    // @seealso group
    class_addmethod(c, (method) roll_ungroup, "ungroup", A_GIMME, 0);

    
    
	// @method onset @digest Modify the onset of selected items
	// @description The word <m>onset</m>, followed by a number, sets the new onset, in milliseconds, for all 
	// the selected notation items. This absolute onset can be replaced by an llll containing a relative modification of the
	// existing onset, or by a generic equation.
	// @copy BACH_DOC_RELATIVE_MODIFICATIONS
	// @copy BACH_DOC_EQUATION_MODIFICATIONS 
	// @marg 0 @name onset @optional 0 @type number/llll/anything
    // @example onset 1000 @caption move selected items to 1s
    // @example onset = 1000 @caption exactly the same
    // @example onset = onset + 1000 @caption shift selected items by 1s forward
    // @example onset = (chordindex - 1) * 1000 @caption space all chords by 1s
    // @example onset = "(pow(1.4, chordindex) - 1)*1000" @caption the same, in rallentando
    // @seealso distribute
	class_addmethod(c, (method) roll_sel_change_onset, "onset", A_GIMME, 0);

   	class_addmethod(c, (method) roll_sel_change_ioi, "ioi", A_GIMME, 0); // this one's undocumented for now

	// @method duration @digest Modify the duration of selected items
	// @description The word <m>duration</m>, followed by a number, sets the new duration, in milliseconds, for all 
	// the selected notation items. 
	// If a list of numbers is given as arguments, this is applied to selected chords notewise: bottommost note will be assigned
	// the first value, the one above will be assigned the next one, and so on. If less values than selected notes in the chord 
	// are given, last value is padded.
	// Any of the numbers can be replaced by an llll containing a relative modification of the existing duration. 
	// If a single number is inserted, this can be replaced by a generic equation.
	// @copy BACH_DOC_RELATIVE_MODIFICATIONS
	// @copy BACH_DOC_EQUATION_MODIFICATIONS 
	// @marg 0 @name duration @optional 0 @type number/llll/anything
    // @example duration 1000 @caption change duration of selected items to 1s
    // @example duration = 1000 @caption exactly the same
    // @example duration = duration - 1000 @caption reduce duration by 1s
    // @example duration = 30000 - onset @caption make selection end together at 30s
    // @example duration = velocity * 2 @caption assign durations depending on velocity
    // @seealso tail, legato
	class_addmethod(c, (method) roll_sel_change_duration, "duration", A_GIMME, 0);


	// @method tail @digest Modify the tail position of selected items
	// @description The word <m>tail</m>, followed by a number, sets the new position of the tail, in milliseconds, for all 
	// the selected notation items. Such position must fall, for every chord, after the chord onset, otherwise it is clipped to the chord onset 
	// (no chords with negative durations are allowed).
	// If a list of numbers is given as arguments, this is applied to selected chords notewise: bottommost note will be assigned
	// the first value, the one above will be assigned the next one, and so on. If less values than selected notes in the chord 
	// are given, last value is padded.
	// Any of the numbers can be replaced by an llll containing a relative modification of the existing tail position. 
	// If a single number is inserted, this can be replaced by a generic equation.
	// @copy BACH_DOC_RELATIVE_MODIFICATIONS
	// @copy BACH_DOC_EQUATION_MODIFICATIONS 
	// @marg 0 @name tail_position_ms @optional 0 @type number/llll/anything
    // @example tail 1000 @caption set selected items' tail at 1s
    // @example tail = 1000 @caption exactly the same
    // @example tail = tail + 1000 @caption lengthen selected notes by 1s
    // @example tail = 30000 @caption make selection end together at 30s
    // @example tail = "onset + random(0, 1000)" @caption assign duration randomly
    // @seealso duration, legato
	class_addmethod(c, (method) roll_sel_change_tail, "tail", A_GIMME, 0);


	// @method velocity @digest Modify the velocity of selected items
	// @description @copy BACH_DOC_MESSAGE_VELOCITY
	// @marg 0 @name velocity @optional 0 @type int/llll/anything
    // @example velocity 120 @caption set selected items' velocity to 120
    // @example velocity = 120 @caption exactly the same
    // @example velocity = velocity * 1.2 @caption increase velocity
    // @example velocity = duration/1000. @caption assign velocity depending on duration
    // @example velocity = "random(40, 121)" @caption assign velocity randomly
    class_addmethod(c, (method) roll_sel_change_velocity, "velocity", A_GIMME, 0);


	// @method cents @digest Modify the cents of selected items
	// @description @copy BACH_DOC_MESSAGE_CENTS
	// @marg 0 @name cents @optional 0 @type number/llll/anything
    // @example cents 6000 @caption change selected notes to middle C's
    // @example cents = 6000 @caption exactly the same
    // @example cents = cents * 1.2 @caption stretch pitches
    // @example cents = "random(48, 73)*100" @caption assign pitch randomly
    // @example cents = 6000 + (cents % 1200) @caption collapse to middle octave
	class_addmethod(c, (method) roll_sel_change_cents, "cents", A_GIMME, 0);

    
    // @method pitch @digest Modify the pitch of selected items
    // @description @copy BACH_DOC_MESSAGE_PITCH
    // @marg 0 @name pitch @optional 0 @type number/llll/anything
    // @example pitch C5 @caption change selected notes to middle C's
    // @example pitch = C5 @caption exactly the same
    // @example pitch = pitch + D0 @caption transpose by major second
    // @example pitch = pitch + Eb0 @caption transpose by minor third
    // @example pitch = pitch - C1 @caption transpose one octave down
    // @example pitch = (pitch % C1) + C5 @caption collapse to middle octave
    class_addmethod(c, (method) roll_sel_change_pitch, "pitch", A_GIMME, 0);
    
	
	// @method voice @digest Modify the voice of selected items
	// @description @copy BACH_DOC_MESSAGE_VOICE
	// @marg 0 @name voice_number @optional 0 @type int/llll/anything
    // @example voice 2 @caption put selected notes in second voice
    // @example voice = 2 @caption exactly the same
    // @example voice = voice + 1 @caption shift voices
    // @example voice = "random(1, 4)" @caption assign voice randomly between 1 and 3
    // @example voice = (3 - voice)*(voice <= 2) + voice * (voice > 2) @caption swap first two voices
    // @see insertvoice
	class_addmethod(c, (method) roll_sel_change_voice, "voice", A_GIMME, 0);
	

	// @method addbreakpoint @digest Add a pitch breakpoint to each selected note
	// @description @copy BACH_DOC_MESSAGE_ADDBREAKPOINT
	// @marg 0 @name relative_x_position @optional 0 @type float
	// @marg 1 @name delta_midicents @optional 0 @type float
	// @marg 2 @name slope @optional 1 @type float
	// @marg 3 @name velocity @optional 1 @type int
    // @example addbreakpoint 0.5 200 @caption add a breakpoint at the middle of the note of +200cents
    // @example addbreakpoint 0.5 200 0.2 @caption the same, with slope 0.2
    // @example addbreakpoint 0.5 200 0.2 80 @caption the same, with velocity 80
    // @see erasebreakpoints
	class_addmethod(c, (method) roll_sel_add_breakpoint, "addbreakpoint", A_GIMME, 0);
	
	
	// @method erasebreakpoints @digest Delete pitch breakpoints of selected notes
	// @description @copy BACH_DOC_MESSAGE_ERASEBREAKPOINTS
    // @see addbreakpoint
	class_addmethod(c, (method) roll_sel_erase_breakpoints, "erasebreakpoints", A_GIMME, 0);
	
	
	// @method addslot @digest Set the content of one or more slots for selected items
	// @description @copy BACH_DOC_MESSAGE_ADDSLOT
	// @marg 0 @name slots @optional 0 @type llll
    // @example addslot (6 0.512) @caption fill (float) slot 6 with number 0.512
    // @example addslot (5 42) @caption fill (int) slot 5 with number 42
    // @example addslot (7 "Lorem Ipsum" ) @caption fill (text) slot 7 with some text
    // @example addslot (10 (John George (Ringo) (Brian)) ) @caption fill (llll) slot 10 with an llll
    // @example addslot (3 10 20 30) @caption fill (intlist) slot 3 of selected notes with list of values 10, 20, 30
    // @example addslot (2 (0 0 0) (0.5 0 1) (1 1 0.2) @caption fill (function) slot 2 with a breakpoint function in (x y slope) form
    // @example addslot (amplienv (0 0 0) (0.5 0 1) (1 1 0.2)) @caption the same for slot named 'amplienv'
    // @example addslot (active (0 0 0) (0.5 0 1) (1 1 0.2)) @caption the same for currently open slot
    // @example addslot (3 10 20 30) (2 (0 0 0) (0.5 0 1) (1 1 0.2)) @caption set more slots at once
    // @seealso changeslotvalue, eraseslot
	class_addmethod(c, (method) roll_sel_add_slot, "addslot", A_GIMME, 0);


	// @method eraseslot @digest Clear a specific slot for selected items
	// @description @copy BACH_DOC_MESSAGE_ERASESLOT 
	// @marg 0 @name slot_number_or_name @optional 0 @type int/symbol
    // @example eraseslot active @caption clear currently open slot for selected items
    // @example eraseslot 4 @caption clear 4th slot
    // @example eraseslot amplienv @caption clear slot named amplienv
    // @seealso copyslot, moveslot, addslot, changeslotvalue, resetslotinfo
	class_addmethod(c, (method) roll_sel_erase_slot, "eraseslot", A_GIMME, 0);

    
    // @method moveslot @digest Move a slot to another one for selected items
    // @description @copy BACH_DOC_MESSAGE_MOVESLOT
    // @marg 0 @name slot_from @optional 0 @type int/symbol
    // @marg 1 @name slot_to @optional 0 @type int/symbol
    // @example moveslot 2 7 @caption move the content of slot 2 to slot 7 for selected items
    // @example moveslot 2 active @caption destination slot is the active slot
    // @example copyslot amplienv myfunction @caption copy the slot named amplienv to the slot named myfunction
    // @seealso copyslot, eraseslot, addslot, changeslotvalue, resetslotinfo
    class_addmethod(c, (method) roll_sel_move_slot, "moveslot", A_GIMME, 0);

    
    // @method copyslot @digest Copy a slot to another one for selected items
    // @description @copy BACH_DOC_MESSAGE_COPYSLOT
    // @marg 0 @name slot_from @optional 0 @type int/symbol
    // @marg 1 @name slot_to @optional 0 @type int/symbol
    // @example copyslot 2 7 @caption copy the content of slot 2 to slot 7 for selected items
    // @example copyslot 2 active @caption destination slot is the active slot
    // @example copyslot amplienv myfunction @caption copy the 'amplienv' slot to the 'myfunction' slot
    // @seealso moveslot, eraseslot, addslot, changeslotvalue, resetslotinfo
    class_addmethod(c, (method) roll_sel_copy_slot, "copyslot", A_GIMME, 0);


	// @method changeslotvalue @digest Change a specific value inside a slot for selected items
	// @description @copy BACH_DOC_MESSAGE_CHANGESLOTVALUE
	// @marg 0 @name slot_number_or_name @optional 0 @type int/symbol
	// @marg 1 @name element_index @optional 0 @type int
	// @marg 2 @name slot_element @optional 0 @type llll	
    // @example changeslotvalue 3 2 13 @caption set the 2nd element of 3nd (int or float) slot to 13
    // @example changeslotvalue 3 0 13 @caption append 13 at the end of slot 3
    // @example changeslotvalue 1 2 0.5 10 0 @caption change the 2nd point of the 1st (function) slot to (0.5 10 0) in (x y slope) form
    // @example changeslotvalue active 2 0.5 10 0 @caption the same, for the currently open slot
    // @example changeslotvalue spectrenv 2 0.5 10 0 @caption the same, for the a slot named 'spectrenv'
    // @example changeslotvalue 9 1 highpass 400 0 2 @caption set the 1st element of 9nd (dynfilter) slot to "highpass 400 0 2"
    // @example changeslotvalue 8 0 Max.app 0 @caption append the Max.app file in the 8th (filelist) slot, and make it active
    // @example changeslotvalue 8 0 0 2 @caption Make 2nd file active in 8th (filelist) slot
    // @seealso addslot, eraseslot
	class_addmethod(c, (method) roll_sel_change_slot_value, "changeslotvalue", A_GIMME, 0);
	
	
	// @method dumpselection @digest Play selected items off-line
	// @description The <m>dumpselection</m> message sends the content of each one of selected notation items from the 
	// playout, in playout syntax (off-line play).
    // You can safely rely on the fact that elements will be output ordered by onset. <br />
    // If a "router" message attribute is set, then the standard router ("note", "chord") is replaced by the specified one;
    // if the "router" attribute has length 2, the first symbol will be used for notes, the second one for chords. <br />
	// @copy BACH_DOC_PLAYOUT_SYNTAX_ROLL
    // @mattr router @type llll @default null @digest Sets a forced router to be used instead of the default one
    // @seealso sendcommand, play
	class_addmethod(c, (method) roll_sel_dumpselection, "dumpselection", A_GIMME, 0);


	// @method sendcommand @digest Send a command to selected items
	// @description The <m>sendcommand</m> message sends a given command to the selected notation items,
	// whose content is then immediately output from the playout in playout syntax (see <m>dumpselection</m>)
	// but with a router different from the standard playout syntax router, i.e. the router defined by the given command.
	// It expects as argument the number of the command to be sent (1 to 5).
    // You can safely rely on the fact that elements will be output ordered by onset.
	// @copy BACH_DOC_COMMANDS_GENERAL
	// @copy BACH_DOC_PLAYOUT_SYNTAX_ROLL
	// @marg 0 @name command_number @optional 0 @type int
    // @example sendcommand 3 @caption send 3rd command
    // @seealso dumpselection
	class_addmethod(c, (method) roll_sel_sendcommand, "sendcommand", A_GIMME, 0);
	
	
	// @method snappitchtogrid @digest Snap selected notes' pitches to the current microtonal grid
	// @description @copy BACH_DOC_MESSAGE_SNAPPITCHTOGRID
    // @seealso respell, snaponsettogrid, snaptailtogrid
	class_addmethod(c, (method) roll_sel_snap_pitch_to_grid, "snappitchtogrid", 0);


	// @method snaponsettogrid @digest Snap selected chords' onsets to the current temporal grid
	// @description The <m>snaponsettogrid</m> message snaps the onset of each selected chord (or 
	// of each chord in which at least one note is selected) to the current temporal grid (if any).
	// Also see the <m>grid</m> attribute.
    // @seealso snappitchtogrid, snaptailtogrid
	class_addmethod(c, (method) roll_sel_snap_onset_to_grid, "snaponsettogrid", 0);


	// @method snaptailtogrid @digest Snap selected notes' tails to the current temporal grid
	// @description The <m>snaptailtogrid</m> message snaps the tails of each selected note to the current temporal grid (if any).
	// Also see the <m>grid</m> attribute.
    // @seealso snappitchtogrid, snaponsettogrid
	class_addmethod(c, (method) roll_sel_snap_tail_to_grid, "snaptailtogrid", 0);


	// @method respell @digest Respell selected notes automatically
	// @description @copy BACH_DOC_MESSAGE_RESPELL
    // @seealso snappitchtogrid
	class_addmethod(c, (method) roll_sel_resetgraphic, "respell", 0);


	// @method legato @digest Make selection legato
	// @description The <m>legato</m> message shifts the tails of each one of the selected notes so that it reaches exactly the
	// onset of the following chord, resulting in a sequence of chords where each one ends when the next one begins. <br />
    // If the <b>trim</b> symbol is given as first argument, the legato will only shorten the notes duration, hence preserving
    // all rests between chords. If the <b>extend</b> symbol is given as first argument, the legato will only make note durations longer,
    // hence preserving all superposition of notes across chords.
    // @marg 0 @name trim_or_extend @optional 1 @type symbol
    // @example legato @caption make selection monophonic, with no rests
    // @example legato trim @caption the same, preserving rests
    // @example legato extend @caption the same, preserving notes superpositions
    // @seealso tail, duration, glissando
	class_addmethod(c, (method) roll_legato, "legato", A_GIMME, 0);


    // @method glissando @digest Make selection glissando
    // @description The <m>glissando</m> message works exactly like the <m>legato</m> message, but in addition to that
    // modifies the midicents of the notes' tails so that they match the next notehead pitch (yielding continuous glissandi across
    // selected notes). <br />
    // If the <b>trim</b> symbol is given as first argument, the glissando will only shorten the notes duration, hence preserving
    // all rests between chords. If the <b>extend</b> symbol is given as first argument, the glissando will only make note durations longer,
    // hence preserving all superposition of notes across chords. <br />
    // Another optional argument (after the possible symbol) is the glissando slope, as floating point value, from -1. to 1. (0. being linear, default).
    // @marg 0 @name trim_or_extend @optional 1 @type symbol
    // @marg 1 @name slope @optional 1 @type float
    // @example glissando @caption make selection monophonic, with no rests, and continuously glissando
    // @example glissando 0.5 @caption the same, with a 0.5 glissando slope
    // @example glissando trim 0.5 @caption the same, preserving rests
    // @example glissando extend 0.5 @caption the same, preserving notes superpositions
    // @seealso tail, duration, glissando
    class_addmethod(c, (method) roll_glissando, "glissando", A_GIMME, 0);


	// @method clearselection @digest Clear current selection
	// @description @copy BACH_DOC_MESSAGE_CLEARSELECTION
	// @marg 0 @name voice_number @optional 1 @type int
    // @example clearselection @caption make no elements selected
    // @example clearselection 3 @caption make no elements selected in voice 3
    // @seealso sel, unsel, select
	class_addmethod(c, (method) roll_clearselection, "clearselection", A_GIMME, 0);


	// @method getcurrentchord @digest Get notes at cursor position
	// @description @copy BACH_DOC_MESSAGE_GETCURRENTCHORD 
    // @seealso interp, sample
	class_addmethod(c, (method) roll_send_current_chord, "getcurrentchord", 0);


	// @method explodechords @digest Turn all chords into 1-note-chords
	// @description The <m>explodechords</m> message turns any chord having more than 1 note
	// in a sequence of overlapping chords each having just one note. In the end, the number
	// of notes and the number of chords will coincide.
    // @example explodechords @caption Explode chords for whole score
    // @example explodechords selection @caption Explode selected chords only
    // @seealso merge
	class_addmethod(c, (method) roll_anything, "explodechords", A_GIMME, 0);


	// @method adjustadditionalstartpad @digest Adjust additional start pad automatically 
	// @description The <m>adjustadditionalstartpad</m> adjusts automatically the additional
	// padding space at the beginning of the <o>bach.roll</o>, so that all accidentals
	// of the notes near the beginning can be properly displayed. Also see the <m>additionalstartpad</m> attribute.
	class_addmethod(c, (method) roll_adjustadditionalstartpad, "adjustadditionalstartpad", 0);


	// @method fixvzoom @digest Fix the vertical zoom value 
	// @description @copy BACH_DOC_MESSAGE_FIXVZOOM
    // @seealso getvzoom
	class_addmethod(c, (method) roll_fixvzoom, "fixvzoom", 0);


	// @method addmarker @digest Add a marker 
	// @description The <m>addmarker</m> adds a marker with a specified name at a specified position in the score.
	// At least two arguments are needed: the position of the marker in milliseconds (or the "cursor" symbol) and the marker name, as symbol.
	// For instance, <b>addmarker 1000 foo</b> adds a marker at 1000ms with the name "foo".
	// If more than one name need to be associated to the marker, the symbol can be replaced by a list of symbols,
	// for instance <b>addmarker 1000 (foo fee)</b>. If no names need to be associated to the marker, leave <b>()</b> as
	// name llll. The millisecond position can be replaced by the "cursor" symbol or by the "end" symbol (end of score). <br />
	// If the marker has a role (see below), this should be stated as symbol as third argument.	The choice is among the 
	// following symbols: "none" (default: no role), "timesig" (time signature), "tempo", "barline" and "division".
	// If the specified role requires a content, this should be indicated as fourth argument. For instance
	// <b>addmarker 0 (not important) timesig (4 4)</b> adds a marker at the beginning of the <o>bach.roll</o>, having a
	// time signature role, corresponding to a 4/4 time signature. <br /> <br />
	// @copy BACH_DOC_MARKERROLES
	// @marg 0 @name position @optional 0 @type float/symbol
	// @marg 1 @name name_or_names @optional 0 @type llll
	// @marg 2 @name role @optional 1 @type symbol
	// @marg 3 @name content @optional 1 @type llll
    // @example addmarker 2000 George @caption add a marker named 'George' at 2s
    // @example addmarker 2000 (Foo Fee Faa) @caption the same, with a complex llll name
    // @example addmarker cursor John @caption add a 'John' marker at the current playhead position
    // @example addmarker 1000 whatever barline @caption add a barline marker at 1s
    // @example addmarker 500 whatever division @caption add a division marker at 0.5s
    // @example addmarker 0 whatever timesig (2 4) @caption add a 2/4 time signature marker at the beginning
    // @example addmarker 0 whatever tempo (1/4 120) @caption add a quarter = 120 tempo marker at the beginning
    // @seealso deletemarker, clearmarkers
	class_addmethod(c, (method) roll_addmarker, "addmarker", A_GIMME, 0);


	// @method deletemarker @digest Delete a marker 
	// @description @copy BACH_DOC_MESSAGE_DELETEMARKER
	// @marg 0 @name marker_names @optional 0 @type list
    // @example deletemarker Ringo @caption deletes first marker named Ringo
    // @example deletemarker Ringo Starr @caption deletes first marker with both Ringo and Starr as names
    // @seealso addmarker, clearmarkers, delete
	class_addmethod(c, (method) roll_deletemarker, "deletemarker", A_GIMME, 0);


	// @method clearmarkers @digest Delete all markers
	// @description @copy BACH_DOC_MESSAGE_CLEARMARKERS
    // @seealso addmarker, deletemarker
	class_addmethod(c, (method) roll_clearmarkers, "clearmarkers", 0);


	// @method getmarker @digest Retrieve marker information
	// @description The <m>getmarker</m>, without any further argument, will output all the markers from the playout in the form 
	// <b>markers <m>MARKER1</m> <m>MARKER2</m>...</b>, where each <m>MARKER</m> is an llll of the form
	// <b>(<m>position_ms</m> <m>name_or_names</m> <m>role</m> <m>optional:content</m>)</b>, where the <m>content</m> is only output
	// if the marker <m>role</m> requires it (see below to know more about marker roles). 
	// Markers are in any case always output ordered according to their positions.
	// The <m>name_or_names</m> parameter is either a single symbol or integer (if the marker has a single name), or an llll containing all the names
	// listed, in the form <b>(<m>name1</m> <m>name2</m> ...)</b>, where each <m>name</m> is a symbol or an integer. If a marker has no name, 
	// then <b>()</b> is used.  <br />
	// If you send a message <b>getmarker @namefirst 1</b>, all the markers will be output from the playout in the form
	// <b>markers <m>MARKER1</m> <m>MARKER2</m>...</b>, where each <m>MARKER</m> is an llll of the form
	// <b>(<m>name_or_names</m> <m>position_ms</m> <m>role</m> <m>optional:content</m>)</b>. <br />
	// You can retrieve the information about a specific marker by adding the marker name or names as arguments. In this case you'll get
	// from the playout an llll in the form <b>marker <m>name_or_names</m> <m>position_ms</m> <m>role</m> <m>optional:content</m></b>, where all
	// the parameters are the same as above.
	// If more than one marker match the introduced name(s), only the information about the first marker (in temporal order) is output.
	// <br /> <br />
	// @copy BACH_DOC_MARKERROLES
	// @marg 0 @name names @optional 1 @type list
    // @mattr namefirst @type int @default 0 @digest If non-zero outputs the marker name(s) as first element
    // @example getmarker @caption get all markers in standard (position name role content) form
    // @example getmarker @namefirst 1 @caption get all markers in (name position role content) form
    // @example getmarker Ringo @caption get the first marker named 'Ringo'
    // @example getmarker Ringo Starr @caption get the first marker named both 'Ringo' and 'Starr'
    // @seealso dump
	class_addmethod(c, (method) roll_getmarker, "getmarker", A_GIMME, 0);
	
	class_addmethod(c, (method) roll_markername, "markername", A_GIMME, 0); // deprecated: use "name"


	// @method setcursor @digest Move the playhead cursor 
	// @description The <m>setcursor</m> message moves the playhead cursor to a specific position, given as argument in milliseconds.
	// This can be replaced by a symbol or a list of symbols containing the name(s) of a marker, whose position has to be the playhead cursor position.
	// If more than one markers match the name(s), the cursor will be moved to the first one.
	// @marg 0 @name position @optional 0 @type list
    // @example setcursor 2000 @caption set the playhead at 2s
    // @example setcursor John @caption set the cursor at the onset of the first item named 'John'
    // @example setcursor John Lennon @caption set the cursor at the onset of the first item named 'John' and 'Lennon'
    // @seealso getcursor, showcursor, hidecursor
	class_addmethod(c, (method) roll_setcursor, "setcursor", A_GIMME, 0);


	// @method getcursor @digest Get current playhead cursor position 
	// @description The <m>getcursor</m> message retrieves the current playhead cursor position, and outputs it from the playout in the form
	// <b>cursor <m>cursor_position_ms</m></b>, where the last parameter is the current position of the playhead cursor in milliseconds. 
    // @seealso setcursor, showcursor, hidecursor
	class_addmethod(c, (method) roll_getcursor, "getcursor", 0);
	
    
    // @method hidecursor @digest Hide the playhead cursor
    // @description @copy BACH_DOC_MESSAGE_HIDECURSOR
    // @seealso showcursor, getcursor, setcursor
    class_addmethod(c, (method) roll_hidecursor, "hidecursor", 0);
    
    
    // @method showcursor @digest Show the playhead cursor
    // @description @copy BACH_DOC_MESSAGE_SHOWCURSOR
    // @seealso hidecursor, getcursor, setcursor
    class_addmethod(c, (method) roll_showcursor, "showcursor", 0);
    


	// @method getloop @digest Get current loop region position 
	// @description The <m>getloop</m> message retrieves the current loop region position, and outputs it from the playout in the form
	// <b>loop <m>loop_start_ms</m> <m>loop_end_ms</m></b>, where the two last parameter
	// are the beginning and end position of the loop, in milliseconds (also see the <m>loop</m> attribute). 
	class_addmethod(c, (method) roll_getloop, "getloop", 0);


	// @method quantize @digest Send quantization data 
	// @description The <m>quantize</m> message outputs all the content of <o>bach.roll</o> from the first outlet,
	// in a form which is understandable by <o>bach.quantize</o>.
	// You should just connect the first outlet of <o>bach.roll</o> to the second inlet of <o>bach.quantize</o>, to have this latter
	// properly get all the information (refer to the <o>bach.quantize</o> help and documentation; also see #quantize in the help center). <br />
	// By default, the <m>quantize</m> message also outputs all the header information, but as for the <m>dump</m> message, 
	// if one needs to only dump specific header elements, the <m>quantize</m> message may also accept a list of symbols referring to them.
	// Namely, the possibilities are the following ones: "keys", "clefs", "voicenames", "midichannels", "markers", "slotinfo", "command", "groups".
	// Also the "body" symbol should always be added, in order to also dump the music content (otherwise no real quantization can be performed).
	// For instance, <b>quantize keys slotinfo body</b> will dump the key signatures, the slotinfo and will send the quantization data of the 
	// music content.
	// @marg 0 @name selective_quantize_options @optional 1 @type list
    // @example quantize @content quantize whole content
    // @example quantize body @content quantize but keep no header elements
    // @example quantize body keys markers @content as previously, but also keep keys and markers
	class_addmethod(c, (method) roll_quantize, "quantize", A_GIMME, 0);


	// @method lambda @digest Perform operation on individual selected items 
	// @description @copy BACH_DOC_MESSAGE_LAMBDA
	// @marg 0 @name modification_message @optional 0 @type llll
    // @example lamdba cents $1 @caption assign the incoming value as note cents
    // @example lamdba changeslotvalue $1 $2 $3 @caption the same, for some slot value
    // @seealso cents, velocity, duration, onset, changeslotvalue, addslot, eraseslot, name, voice
	class_addmethod(c, (method) roll_lambda, "lambda", A_GIMME, 0);


	// @method clear @digest Clear all the content or a specific parameter inlet
	// @description A <m>clear</m> message sent in the first inlet will delete all the chords of the <o>bach.roll</o>, and all the markers.
	// If an integer argument is given, the message will only clear the content of a specific voice (the one corresponding to the input integer number). <br />
	// A <m>clear</m> message sent in any of the separate parameters inlets (all inlets but the first one) will clear the content which was
	// possibly stored in such inlet. This is equivalent to sending a <b>nil</b> or <b>()</b> message in that inlet.
	// @marg 0 @name voice_number @optional 1 @type int
    // @example clear @caption clear all items
    // @example clear 3 @caption the same, only for voice 3
    // @seealso clearall, clearbreakpoints
	class_addmethod(c, (method) roll_anything, "clear", A_GIMME, 0);


	// @method clearall @digest Clear all separate parameters inlets
	// @description @copy BACH_DOC_MESSAGE_CLEARALL
    // @seealso clear
	class_addmethod(c, (method) roll_anything, "clearall", A_GIMME, 0);

    
    // @method clearbreakpoints @digest Clear all the pitch breakpoints
    // @description A <m>clearbreakpoints</m> message sent in the first inlet will delete all the pitch breakpoints inside the <o>bach.roll</o>, and
    // reset the note tails.
    // If an integer argument is given, the message will only clear the content of a specific voice (the one corresponding to the input integer number). <br />
    // @marg 0 @name voice_number @optional 1 @type int
    // @example clear @caption clear all pitch breakpoints
    // @example clear 3 @caption the same, only for voice 3
    // @seealso clear
    class_addmethod(c, (method) roll_anything, "clearbreakpoints", A_GIMME, 0);

	
	// @method interp @digest Obtain active notes data at a given instant
	// @description The <m>interp</m> message, followed by a time value (in milliseconds) retrieves
	// the instantaneous data of all the notes which are active at the given time instant.
	// The answer is sent through the playout in the following form: <b>interp <m>VOICE1</m> <m>VOICE2</m>...</b>
	// where each <m>VOICE</m> is in the form <b>(<m>CHORD1</m> <m>CHORD2</m>...)</b>, being the chords active at the 
	// given time instant, each in the form <b>(<m>NOTE1</m> <m>NOTE2</m>...)</b>, being the chord notes active at
	// the given time instant, each in the standard note gathered syntax, with two important variations:
	// there is no duration element, and for each slot marked as temporal only the slot element at the given time instant
	// is output (e.g. the interpolated function point of a function slot). <br /> <br />
	// @copy BACH_DOC_NOTE_GATHERED_SYNTAX_ROLL
	// @marg 0 @name time @optional 0 @type number
    // @example interp 1000 @caption get info on chords being played at 1s
    // @seealso getcurrentchord, sample
	class_addmethod(c, (method) roll_anything, "interp", A_GIMME, 0);
	
	
	// @method sample @digest Sample score data
	// @description The <m>sample</m> message, followed by a integer (the number of sampling points), 
	// samples the note data (exactly as <m>interp</m> does) throughout the score, at the (uniformly taken) sampling point. 
	// The answer is sent through the playout in the following form: <b>sample (<m>t1</m> <m>t2</m>...) (<m>RES1</m> <m>RES2</m>...)...</b>
	// where each <m>t</m> is an instant in milliseconds, and each <m>RES</m> is the result of the <m>interp</m> message performed
	// on such instant (see <m>interp</m> to know more). <br />
    // If the <b>ms</b> symbol is given as second argument, the first numeric argument (which can also be non-integer, in
    // this case) is considered to be the distance between samples (in milliseconds), and not the number of samples.
	// @marg 0 @name num_samples @optional 0 @type number
    // @marg 1 @name ms @optional 1 @type symbol
    // @example sample 10 @caption sample score in 10 equally spaced points
    // @example sample 1000 ms @caption sample score each second
    // @seealso interp, getcurrentchord
	class_addmethod(c, (method) roll_anything, "sample", A_GIMME, 0);
	
	
	// @method slice @digest Slice all the notes at a given instant 
	// @description A <m>slice</m> message followed by a number (considered a position in milliseconds) will split all the notes 
	// inside the <b>bach.roll</b> which exist at the given position into two notes: a left- and a right-sided note. All breakpoints and temporal slots will be 
	// properly split accordingly. The argument can also be a wrapped llll of numbers, in which case a sequence of slice will be performed at different positions.
	// Finally, one can give as a second argument the wrapped list of voices whose chords have to be slice. If such argument is not given, or if it is empty,
	// all voices will be slice by default.
	// @marg 0 @name positions @optional 0 @type number/llll
	// @marg 1 @name voices @optional 1 @type llll
    // @example slice 1000 @caption slice score at 1s
    // @example slice (2000 3000 5000) @caption slice score at 1s, 2s and 5s
    // @example slice (2000 3000 5000) (1 3) @caption the same, but only voices 1 and 3
	class_addmethod(c, (method) roll_slice, "slice", A_GIMME, 0);
	
	
	// @method dump @digest Dump information
	// @description A <m>dump</m> message will simply output 
	// the content of all separate parameters from all separate outlets (in separate syntax, right-to-left), and 
	// all the content of <o>bach.roll</o> in gathered syntax from the first outlet (header information included). See below to know more
	// about gathered and separate syntaxes. <br />
	// The <m>dump</m> message also accepts as argument one of the following symbols, which will only dump a portion of the global information: <br />
	// - <b>onsets</b>: only dump the onsets (in separate syntax) from the second outlet. <br />
	// - <b>cents</b>: only dump the cents (in separate syntax) from the third outlet. <br />
	// - <b>durations</b>: only dump the durations (in separate syntax) from the fourth outlet. <br />
	// - <b>velocities</b>: only dump the velocities (in separate syntax) from the fifth outlet. <br />
	// - <b>extras</b>: only dump the extras (in separate syntax) from the sixth outlet. <br />
	// - <b>roll</b>: only dump the whole <o>bach.roll</o> gathered syntax from the first outlet (and nothing from the separate parameters outlets). <br />
	// - <b>body</b>: only dump the whole <o>bach.roll</o> gathered syntax from the first outlet, and drop all the header specifications: only dump the 
	// body (content) of the <o>bach.roll</o>. <br />
	// - <b>header</b>: only dump the whole <o>bach.roll</o> from the first outlet, dropping all the content (body) specification, and only 
	// outputting all the header specification. <br />
	// Moreover, if one needs to only dump specific header elements, the <m>dump</m> message may also accept a list of symbols referring to them.
	// Namely, the possibilities are the following ones: "keys", "clefs", "voicenames", "midichannels", "markers", "slotinfo", "command", "groups".
	// Also the "body" symbol can always be added, in order to also dump the music content.
	// For instance, <b>dump keys slotinfo voicenames</b> will dump the key signatures, the slotinfo and the voicenames, 
	// while, <b>dump keys slotinfo voicenames body</b> will do the same but will also output the music content (body). 
	// The order of the dumped elements does not correspond in general to the order of the symbols in the list.
	// <br /> <br />
	// @copy BACH_DOC_ROLL_GATHERED_SYNTAX 
	// @copy BACH_DOC_ROLL_SEPARATE_SYNTAX
	// @copy BACH_DOC_SEPARATE_SYNTAX_EXTRAS
	// @marg 0 @name selective_dump_options @optional 1 @type list
    // @example dump @caption dump whole information from all outlets
    // @example dump separate @caption dump separate-syntax outlets only
    // @example dump roll @caption dump first outlet only (gathered syntax)
    // @example dump onsets @caption dump onsets only
    // @example dump velocities @caption dump velocities only
    // @example dump body @caption dump first outlet only, but without dumping the header
    // @example dump header @caption the same, but only dumping the header
    // @example dump keys clefs body @caption dump keys, clefs and body from first outlet
    // @seealso getmarker, llll
	class_addmethod(c, (method) roll_dump, "dump", A_GIMME, 0);
	
	
	// @method addchord @digest Add a chord
	// @description An <m>addchord</m> message will add a new chord to the existing ones.
	// The first optional integer argument is the number of the voice in which the chord should be placed.
	// Then, the chord must be given in its gathered syntax llll form as argument. For instance, <b>addchord 2 (1000 (6100 1000 50))</b>
	// adds a chord in the second voice having onset 1000 and a single note (pitch 6100 midicents, duration 1000 ms, velocity 50).
	// If no voice number is given, first voice is used by default. <br /> <br />
	// @copy BACH_DOC_CHORD_GATHERED_SYNTAX_ROLL
	// @marg 0 @name voice_number @optional 1 @type int
	// @marg 1 @name chord @optional 0 @type llll
    // @example addchord (1000 (6000 500 50)) @caption add a middle c at 1s, lasting 0.5s and with velocity 50
    // @example addchord 2 (1000 (6000 500 50)) @caption the same, in second voice
    // @example addchord 2 (1000 (6000 500 50) (7200 0.5 50)) @caption add a chord with two notes
    // @example addchord 2 (1000 (6000 500 50 (slots (3 10 20 30))) (7200 0.5 50 (breakpoints (0 0 0) (1 -500 0)))) @caption and with extra information
    // @seealso gluechord, addchords, delete
	class_addmethod(c, (method) roll_anything, "addchord", A_GIMME, 0);


    // @method deletevoice @digest Delete a voice
    // @description A <m>deletevoice</m> message, followed by an integer number <m>N</m>, will delete the <m>N</m>-th voice from the score.
    // @marg 0 @name voice_number @optional 0 @type int
    // @example deletevoice 4 @caption delete 4th voice
    // @seealso insertvoice
    class_addmethod(c, (method) roll_anything, "deletevoice", A_GIMME, 0);

    
    // @method insertvoice @digest Insert a voice
    // @description An <m>insertvoice</m> message, followed by an integer number <m>N</m>, will create a new voice and insert it as the <m>N</m>-th voice
    // of the current score. If a second llll argument is set, this can be either an integer, specifying the number of reference voice from where
    // the measureinfo should be copied (hence the new voice will be filled with empty measures, according to the specified reference voice),
    // or an llll, containing the gathered syntax of the whole new voice, so that the new voice comes also
    // filled with musical content. <br />
    // @copy BACH_DOC_VOICE_GATHERED_SYNTAX_ROLL
    // @marg 0 @name voice_number @optional 0 @type int
    // @marg 1 @name voice_or_ref @optional 1 @type llll
    // @example insertvoice 2 @caption insert a empty voice as 2nd voice
    // @example insertvoice 2 4 @caption also initialize it with the properties of 4th voice
    // @example insertvoice 2 ((125 (6300 105 100)) (457 (7500 405 100)) (780 (7000 405 100))) @caption also fill it with some content
    // @seealso deletevoice
    class_addmethod(c, (method) roll_anything, "insertvoice", A_GIMME, 0);

    
	// @method addchords @digest Add several chords
	// @description An <m>addchords</m> message will add several chords to the existing ones.
	// If the message has no arguments, the chords parameters are supposed to have been inserted through the separate parameter inlets,
	// and the <m>addchords</m> message will essentially act like a <m>bang</m> which in turns will preserve the existing content, 
	// and add the newly introduced chords. Refer to the <m>llll</m> message to know more about separate parameter syntax of the inlets.<br />
    // If the message has arguments, the first optional one is a number setting an offset (in millisecond) for the chords to be added.
    // Further arguments are supposed to be llll form, exactly in in the same syntax as the whole <o>bach.roll</o>
	// gathered syntax (without header specification): one llll for each voice,
	// containing one llll for each chord to add (in chord gathered syntax, also see the <m>addchord</m> message). If for a given voice
	// you don't need to add any chord, just set a <b>()</b> llll.
	// More precisely the expected syntax for the argument is <b>(<m>VOICE1</m> <m>VOICE2</m>...)</b>
	// where each voice is an llll in gathered syntax. <br /> <br />
	// For instance, a valid message would be <b>addchords ((217. (7185. 492. 100)) (971. (6057. 492. 100))) ((1665. (7157. 492. 100)))</b>
	// <br /> <br />
	// @copy BACH_DOC_VOICE_GATHERED_SYNTAX_ROLL
    // @marg 0 @name offset @optional 1 @type number
	// @marg 1 @name chords @optional 1 @type llll
    // @example addchords ((217 (7185 492 100)) (971 (6057 492 100))) ((1665 (7157 492 100))) @caption add some chords in gathered syntax
    // @example addchords 1500 ((217 (7185 492 100)) (971 (6057 492 100))) ((1665 (7157 492 100))) @caption same thing, shifted by 1.5 seconds
    // @seealso addchord
	class_addmethod(c, (method) roll_anything, "addchords", A_GIMME, 0);


	// @method gluechord @digest Glue a chord to the existing ones
	// @description A <m>gluechord</m> message acts exactly like an <m>addchord</m> message, which an important difference:
	// it glues the introduced chord, whenever possible, to existing notes. More precisely: if an already existing note is 
	// the "same" as one of the newly introduced notes, and has the tail "near" the onset of the newly introduced chord, then
	// instead of creating a new note, the existing note is made longer.
	// If a first integer argument is given, this is the voice number to which the new chord should be assigned; if no integer is given as
	// first argument, the first voice will be used by default.
	// Then an llll with the chord gathered syntax is expected. Two more optional final arguments can be given, specifying
	// what the previously used word "same" and "near" mean. Such arguments are indeed two thresholds: a threshold in milliseconds 
	// (notes can be glued only if the existing note tail and the new note onset are less distant than this threshold), and a threshold
	// in midicents (notes can be glued only if the existing note pitch and the new note pitch are less distant than this threshold). <br /> 
	// A fifth argument can be given, representing a smooth period in milliseconds. If this number is greater than 0, then if there
	// are discontinuities at glue point, such discontinuities are eased along the defined number of milliseconds. Such argument can also
	// be defined as "inf" (symbol), in which case the newly added point at the discontinuity is dropped.
	// @copy BACH_DOC_CHORD_GATHERED_SYNTAX_ROLL
	// @marg 0 @name voice_number @optional 1 @type int
	// @marg 1 @name chord @optional 0 @type llll
	// @marg 2 @name threshold_ms @optional 1 @type float
	// @marg 3 @name threshold_cents @optional 1 @type float
	// @marg 4 @name smooth_ms @optional 1 @type atom
    // @example gluechord (3000 (6210 1000 100)) @caption glue the chord to a contiguous one, if any
    // @example gluechord (3000 (6210 1000 100)) 10 30 @caption ...only if they join seamlessly within 10ms and 30cents
    // @seealso addchord
	class_addmethod(c, (method) roll_anything, "gluechord", A_GIMME, 0);


	// @method domain @digest Set the displayed domain
	// @description If the <m>domain</m> message is followed by a single number, this is considered as the size in milliseconds
	// of the domain (i.e. the displayed portion of <o>bach.roll</o>), and such message will change the <m>zoom</m> so that the displayed
	// portion of music has indeed that global duration. <br />
	// If the <m>domain</m> message is followed by a two numbers, these are considered as the starting and ending domain points in milliseconds,
	// and such message will change both the <m>zoom</m> and the scrollbar position so that the displayed
	// portion of music starts from the input starting point and ends at the input ending point. <br />
	// If a third argument is given, this is considered to be an ending pad in pixels (scaled with respect to the <m>vzoom</m>), so that the input ending point actually happens
	// a certain number of pixels before (if pad is positive) or after (is pad is negative) the end of the displayed portion of music.  
	// @marg 0 @name duration_or_starting_point_ms @optional 0 @type float
	// @marg 1 @name ending_point_ms @optional 1 @type float
	// @marg 2 @name ending_pad_pixels @optional 1 @type float
    // @example domain 4000 @caption force the domain to contain 4s
    // @example domain 2000 3000 @caption display the portion between 2s and 3s
    // @example domain 2000 3000 10 @caption ...and leave 10 pixels of ending pad
    // @seealso getdomain, getdomainpixels, inscreen, inscreenpos
	class_addmethod(c, (method) roll_domain, "domain", A_GIMME, 0);


	// @method getdomain @digest Get the current domain
	// @description The <m>getdomain</m> message forces <o>bach.roll</o> to output from the playout the current domain,
	// i.e. the portion of <o>bach.roll</o> currently displayed inside the screen. 
	// The syntax of the output answer is: <b>domain <m>start_ms</m> <m>end_ms</m></b>, where the 
	// two elements following the "domain" symbol are the starting and ending point of the domain in milliseconds. <br />
	// @copy BACH_DOC_QUERY_LABELS
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso getdomainpixels, domain, getlength
	class_addmethod(c, (method) roll_getdomain, "getdomain", A_GIMME, 0);


	// @method getdomainpixels @digest Get the pixel position of the domain
	// @description The <m>getdomainpixels</m> message forces <o>bach.roll</o> to output from the playout the current position
	// (in pixels) of the domain, i.e. the starting (left) and ending (right) pixels of the currently displayed portion of music.
	// These pixels are referred to the notation object box.
	// The syntax of the output answer is: <b>domainpixels <m>start_pixel</m> <m>end_pixel</m></b>, where the 
	// two elements following the "domainpixels" symbol are the leftmost and rightmost horizontal points (in pixels) of the domain. <br />
	// @copy BACH_DOC_QUERY_LABELS
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso getdomain, domain
	class_addmethod(c, (method) roll_getdomainpixels, "getdomainpixels", A_GIMME, 0);


	// @method getlength @digest Get the total length
	// @description @copy BACH_DOC_MESSAGE_GETLENGTH
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso getdomain, getdomainpixels
	class_addmethod(c, (method) roll_getlength, "getlength", A_GIMME, 0);


	// @method getnumvoices @digest Get the number of voices
	// @description The <m>getnumvoices</m> message forces <o>bach.roll</o> to output from the playout the current number of voices.
	// The syntax of the output answer is: <b>numvoices <m>num_voices</m></b>, where the last element is the number of voices. <br />
	// @copy BACH_DOC_QUERY_LABELS
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso getnumchords, getnumnotes
	class_addmethod(c, (method) roll_getnumvoices, "getnumvoices", A_GIMME, 0);


	// @method getnumchords @digest Get the number of chords
	// @description The <m>getnumchords</m> message forces <o>bach.roll</o> to output from the playout the number of chords, for each voice.
	// The syntax of the output answer is: <b>numchords <m>num_chords_voice1</m> <m>num_chords_voice2</m>...</b>, 
	// i.e. after the "numchords" symbol, a list of integers is given, representing the number of chords for each one of the voices. <br />
	// @copy BACH_DOC_QUERY_LABELS
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso getnumvoices, getnumnotes
	class_addmethod(c, (method) roll_getnumchords, "getnumchords", A_GIMME, 0);


	// @method getnumnotes @digest Get the number of notes
	// @description The <m>getnumnotes</m> message forces <o>bach.roll</o> to output from the playout the number of notes, for each chord and for each voice.
	// The syntax of the output answer is: <b>numnotes <m>VOICE1</m> <m>VOICE2</m>...</b>, where each <m>VOICE</m> is an llll of the form
	// <b>(<m>num_notes_chord1</m> <m>num_notes_chord2</m>...)</b>, 
	// i.e. after the "numnotes" symbol, a list of lists is given: each outer list represents a voice, and innerly contains an integer for each one
	// of its chords: such integer is the number of notes of the chord. <br />
	// @copy BACH_DOC_QUERY_LABELS
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso getnumvoices, getnumchords
	class_addmethod(c, (method) roll_getnumnotes, "getnumnotes", A_GIMME, 0);


	// @method timetopixel @digest Convert a time position into a pixel position
	// @description The <m>timetopixel</m> message converts a position given in milliseconds (as argument) into a position given in pixels, representing
	// the pixel at which such time position happens, in the current domain.
	// The output answer is sent through the playout, and its syntax is: <b>pixel <m>pixel_position</m></b>, where the last element
	// is indeed the position in pixels corresponding to the inserted position in milliseconds. <br />
	// @copy BACH_DOC_QUERY_LABELS
	// @marg 0 @name query_label @optional 1 @type llll
	// @marg 1 @name position_ms @optional 0 @type float
    // @example timetopixel 4000 @caption get the pixel position corresponding to 4s
    // @seealso pixeltotime
	class_addmethod(c, (method) roll_timetopixel, "timetopixel", A_GIMME, 0);
    class_addmethod(c, (method) roll_getpixelpos, "getpixelpos", A_GIMME, 0);  // old deprecated function


	// @method pixeltotime @digest Convert a pixel position into a time position
	// @description The <m>pixeltotime</m> message converts a position given in pixels into a position given in milliseconds, representing
	// the time corresponding to the inserted pixel position.
	// The output answer is sent through the playout, and its syntax is: <b>time <m>time_position_ms</m></b>, where the last element
	// is indeed the time position (in milliseconds) corresponding to the inserted pixel position. <br />
	// @copy BACH_DOC_QUERY_LABELS
	// @marg 0 @name query_label @optional 1 @type llll
	// @marg 1 @name pixel_position @optional 0 @type float
    // @example pixeltotime 300 @caption get the time position corresponding to 300pixels
    // @seealso timetopixel
	class_addmethod(c, (method) roll_pixeltotime, "pixeltotime", A_GIMME, 0);
    class_addmethod(c, (method) roll_gettimeatpixel, "gettimeatpixel", A_GIMME, 0); // old deprecated function

    
    // @method refresh @digest Force recomputation and redraw
    // @description The <m>refresh</m> message forces the recomputation of all the features and forces the object box to be redrawn.
    class_addmethod(c, (method) roll_anything, "refresh", A_GIMME, 0);

    
    // @method realtime @digest Toggle real-time mode
    // @description The <m>realtime 1</m> message will toggle the real-time mode, i.e. will change some attribute values in order to
    // disable the play highlight, the undo system and the legend. This will increase the object performance for real-time tasks.
    // A subsequent <m>realtime 0</m> message will disable real-time mode, reverting the value of all attribute to the previous ones.
    // @marg 0 @name realtime_mode @optional 0 @type int
    // @example realtime 1 @caption turn real-time mode on
    // @example realtime 0 @caption turn real-time mode off
    class_addmethod(c, (method) roll_anything, "realtime", A_GIMME, 0);

    
	// @method dumpnotepixelpos @digest Retrieve pixel position of every chord and note
	// @description The <m>dumpnotepixelpos</m> message retrieves the pixel position of any chord and note, and outputs it from the playout.
	// The output answer has the syntax: 
	// <b>notepixelpos <m>VOICE1</m> <m>VOICE2</m>...</b>, where each <m>VOICE</m> is an llll of the form
	// <b>(<m>CHORD1</m> <m>CHORD2</m>...)</b>, and where each <m>CHORD</m> is an llll accounting for the pixel position of the chord
	// and all its notes, in the form: 
	// <b>(<m>x-pixel_onset</m> (<m>duration_in_horizontal_pixels_note1</m>  <m>duration_in_horizontal_pixels_note2</m> ...)
	// (<m>y_pixel_noteheadcenter_note1</m>   <m>y_pixel_noteheadcenter_note2</m> ...)
	// (<m>x_pixel_accidental_left_position_note1</m> <m>x_pixel_accidental_left_position_note2</m> ...))</b>.
	// The <m>x-pixel_onset</m> is the pixel corresponding to the chord onset; each <m>duration_in_horizontal_pixels_note</m> is the
	// length of eachnote duration line in pixels; each <m>y_pixel_noteheadcenter_note</m> is the vertical position (in pixels) of the center
	// of the noteheads, and each <m>x_pixel_accidental_left_position_note</m> is the pixel position of the left boundary of the leftmost accidental
	// for each note (or, if a note has no accidentals, the leftmost point of the notehead). <br />
	// @copy BACH_DOC_QUERY_LABELS
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso dumpvoicepixelpos
    class_addmethod(c, (method) roll_dumpnotepixelpos, "dumpnotepixelpos", A_GIMME, 0);
	class_addmethod(c, (method) roll_dumpnotepixelpos, "dumppixelpos", A_GIMME, 0); // old, deprecated
	

	// @method dumpvoicepixelpos @digest Retrieve pixel position of every voice
	// @description @copy BACH_DOC_MESSAGE_DUMPVOICEPIXELPOS
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso dumpnotepixelpos
	class_addmethod(c, (method) roll_dumpvoicepixelpos, "dumpvoicepixelpos", A_GIMME, 0);
    class_addmethod(c, (method) roll_dumpvoicepixelpos, "getvoicepixelpos", A_GIMME, 0);  // old, deprecated
    
    

	// @method getvzoom @digest Retrieve the current vertical zoom
	// @description @copy BACH_DOC_MESSAGE_GETVZOOM
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso getzoom
	class_addmethod(c, (method) roll_getvzoom, "getvzoom", A_GIMME, 0);


	// @method getzoom @digest Retrieve the current horizontal zoom
	// @description @copy BACH_DOC_MESSAGE_GETZOOM
	// @marg 0 @name query_label @optional 1 @type llll
    // @seealso getvzoom, fixvzoom
	class_addmethod(c, (method) roll_getzoom, "getzoom", A_GIMME, 0);


	// @method openslotwin @digest Open a slot window
	// @description @copy BACH_DOC_MESSAGE_OPENSLOTWINS
	// @marg 0 @name slot_number_or_name @optional 0 @type int/symbol
    // @example openslotwin 3 @caption open 3rd slot window for selected note
    // @example openslotwin amplienv @caption open slot window for slot named 'amplienv'
    // @seealso addslot
	class_addmethod(c, (method) roll_openslotwin, "openslotwin", A_GIMME, 0);


    class_addmethod(c, (method) roll_preset, "preset", 0);
    class_addmethod(c, (method) roll_begin_preset, "begin_preset", A_GIMME, 0);
    class_addmethod(c, (method) roll_restore_preset, "restore_preset", A_GIMME, 0);
    class_addmethod(c, (method) roll_end_preset, "end_preset", 0);
	CLASS_METHOD_ATTR_PARSE(c, "begin_preset", "undocumented", gensym("long"), 0L, "1");
	CLASS_METHOD_ATTR_PARSE(c, "restore_preset", "undocumented", gensym("long"), 0L, "1");
	CLASS_METHOD_ATTR_PARSE(c, "end_preset", "undocumented", gensym("long"), 0L, "1");

	class_addmethod(c, (method) roll_int, "int", A_LONG, 0);
	class_addmethod(c, (method) roll_float, "float", A_FLOAT, 0);


	// @method bang @digest Build content from separate parameters
	// @description A <m>bang</m> in any of the inlets will first rebuild new <o>bach.roll</o> content starting from
	// the parameters given as input in the separate inlets (see <m>llll</m> method). 
	// If the <m>autoclear</m> attribute is set to 1, 
	// first of all the <m>bang</m> will clear the content of the <o>bach.roll</o>; if on the other hand
	// the <m>autoclear</m> attribute is set to 0, the new parameter will modify the existing content.
	// Not all the separate parameters need to be given to build the new content: if some are not given, default values
	// will be used instead. Default onsets are 0ms, 1000ms, 2000ms..., but if you input less onsets than needed (e.g. just two)
	// the difference between the last two onsets will be padded in case new chords are to be inserted. Default pitch in cents is 6000 (middle C).
	// Default duration is 500ms, default velocity is 100, but in both cases but the last one is padded if more chords than the input velocities are
	// to be created. Default extras are no extras. 
	class_addmethod(c, (method) roll_bang, "bang", 0);


	// @method llll @digest Function depends on inlet
	// @description In the first inlet, an llll is expected to be a gathered syntax content of the entire <o>bach.roll</o> (building
	// the new <o>bach.roll</o> content from scratch), or an llll containing just some header specification 
	// (thus not affecting the body of the object). See below for more information on the gathered syntax. <br />
	// In any of the other inlets, an llll is expected to contain the separate syntax of some parameter. Namely: <br />
	// - second inlet: Onsets <br />
	// - third inlet: Pitches or cents <br />
	// - fourth inlet: Durations <br />
	// - fifth inlet: Velocities <br />
	// - sixth inlet: Extras <br />
	// See below for more information about the separate syntax. Also see the <m>bang</m> method to rebuild content from these separate
	// parameters.<br /> <br />
	// @copy BACH_DOC_ROLL_GATHERED_SYNTAX
	// @copy BACH_DOC_ROLL_SEPARATE_SYNTAX
	// @copy BACH_DOC_SEPARATE_SYNTAX_EXTRAS
    // @seealso dump
	class_addmethod(c, (method) roll_anything, "anything", A_GIMME, 0);


	// @method play @digest Play
	// @description The <m>play</m> message plays a portion or all the <o>bach.roll</o>.
	// "Playing" means that <o>bach.roll</o> sends from the "playout" outlet, at the timing corresponding to each chord onset,
	// the information about all the chord, or about all the notes of the chord (depending on the <m>playmode</m> attribute, to which you
	// should refer to know more). Markers are also sequenced if the <m>playmarkers</m> attribute is set to 1. 
	// The sequencing takes into account the solo and mute state of each element (see <m>solo</m>, <m>unsolo</m>, <m>mute</m> and <m>unmute</m> messages):
	// if elements are muted, they will not be sequences; if some elements are set as "solo", just those elements will be sequenced.
	// If multiple chords or notes are simultaneous, the order is from topmost voice to bottommost voice, then from lowest note to highest note.
	// Sequencing can be controlled with a variable speed via the <m>clock</m> message and the <o>setclock</o> object.
	// The <m>play</m> message, without any further argument, plays the <o>bach.roll</o> from the current playhead cursor position 
	// (by default: the beginning of the <o>bach.roll</o>) to the end. <br />
	// If you put as first argument the "offline" symbol, all the playing will be done in non-real-time mode, i.e. with no sequencing involved; playing messages
	// will be still output from the playout, but one after another, "immediately". <br />
	// If you give a single numeric argument, it will be the starting point in milliseconds
	// of the region to be played: <o>bach.roll</o> will play from that point to the end. If you give two numeric arguments, they will be the starting and
	// ending point in milliseconds of the region to be played.
	// <br /> <br />
	// @copy BACH_DOC_PLAYOUT_SYNTAX_ROLL
	// @marg 0 @name offline_mode @optional 1 @type symbol
	// @marg 1 @name start_ms @optional 1 @type float
	// @marg 2 @name end_ms @optional 1 @type float
    // @example play @caption play from current playhead position
    // @example play 2000 @caption play starting from 2s till the end
    // @example play 2000 4000 @caption play starting from 2s, stop at 4s
    // @example play offline @caption play in non-realtime mode ("uzi-like")
    // @example play offline 2000 4000 @caption play from 2s to 4s in non-realtime mode
    // @seealso stop, pause, setcursor, playselection
	class_addmethod(c, (method) roll_play, "play", A_GIMME, 0);


	// @method playselection @digest Only play selected items
	// @description The <m>playselection</m> message only plays the selected content. It works exactly like <m>play</m>, but it starts playing
	// at the beginning of the selection, and ends playing at the end of the last selected item. Only selected items are sequenced.
	// Mute and solo status are also taken into account (see <m>play</m>). <br />
	// If you put as first argument the "offset" symbol, all the playing will be done in non-real-time mode, i.e. with no sequencing involved; playing messages
	// will be still output from the playout, but one after another, "immediately", in the low-priority queue. <br />
	// @marg 0 @name offline_mode @optional 1 @type symbol
    // @example playselection @caption play selected items only
    // @example playselection offline @caption the same, in non-realtime mode ("uzi-like")
    // @seealso stop, pause, play
	class_addmethod(c, (method) roll_playselection, "playselection", A_GIMME, 0);
	
	
	// @method stop @digest Stop
	// @description The <m>stop</m> message stops any ongoing playing, if <o>bach.roll</o> was in play mode (nothing happens otherwise).
    // @seealso pause, play, playselection
	class_addmethod(c, (method) roll_stop, "stop", A_GIMME, 0);

	// @method pause @digest Pause
	// @description The <m>pause</m> message pause any ongoing playing, if <o>bach.roll</o> was in play mode (nothing happens otherwise).
	// When paused, the play head cursor is updated to its last reached position while playing, so that a new play will begin by default
	// at that position.
    // @seealso play, playselection, stop
	class_addmethod(c, (method) roll_pause, "pause", A_GIMME, 0);


	// @method mute @digest Mute selected items
	// @description @copy BACH_DOC_MESSAGE_MUTE 
    // @seealso unmute, solo, unsolo
	class_addmethod(c, (method) roll_mute, "mute", 0);


	// @method unmute @digest Unmute selected items
	// @description @copy BACH_DOC_MESSAGE_UNMUTE
    // @seealso mute, solo, unsolo
	class_addmethod(c, (method) roll_unmute, "unmute", 0);


	// @method solo @digest Set selected items as solo
	// @description @copy BACH_DOC_MESSAGE_SOLO
    // @seealso unsolo, mute, unmute
	class_addmethod(c, (method) roll_solo, "solo", 0);


	// @method unsolo @digest Remove solo status from selected items
	// @description @copy BACH_DOC_MESSAGE_UNSOLO
    // @seealso solo, mute, unmute
	class_addmethod(c, (method) roll_unsolo, "unsolo", 0);


	// @method lock @digest Lock selected items
	// @description @copy BACH_DOC_MESSAGE_LOCK
    // @seealso unlock
	class_addmethod(c, (method) roll_lock, "lock", 0);


	// @method unlock @digest Unlock selected items
	// @description @copy BACH_DOC_MESSAGE_UNLOCK
    // @seealso lock
	class_addmethod(c, (method) roll_unlock, "unlock", 0);

	
	// @method copy @digest Copy
	// @description Copies into the global clipboard selected musical content or slot data.
	// If no argument is given, the current selection is copied.
    // If a "durationline" symbol is given as argument, the duration line is copied.
    // If a "slot" symbol is given as argument,
	// the content of the open slot window (if any) is copied; if in addition to the "slot" symbol an integer
	// is given, such integer is the number of the slot whose content is copied (this will work even if no slot window is open);
    // finally, this integer can be substituted by the "all" symbol to copy the content of all slots.
	// @marg 0 @name slot @optional 1 @type symbol
	// @marg 1 @name slot_number @optional 1 @type int/symbol
    // @example copy @caption copy selection
    // @example copy slot 1 @caption copy first slot
    // @example copy slot active @caption copy currently open slot
    // @example copy slot all @caption copy all slots
    // @example copy durationline @caption copy the duration line
    // @seealso cut, paste
	class_addmethod(c, (method) roll_copy, "copy", A_GIMME, 0);

	
	// @method cut @digest Cut
	// @description Cuts, and puts global clipboard, selected musical content or slot data.
	// The behavior and arguments of <m>cut</m> are exaclty as <m>copy</m>, with the addition that the copied 
	// content is subsequently deleted (see <m>copy</m> to know more).
	// @marg 0 @name slot @optional 1 @type symbol
	// @marg 1 @name slot_number @optional 1 @type int/symbol
    // @example cut @caption cut selection
    // @example cut slot 1 @caption cut first slot
    // @example cut slot active @caption cut currently open slot
    // @example cut slot all @caption cut all slot
    // @example cut durationline @caption cut the duration line
    // @seealso copy, paste
	class_addmethod(c, (method) roll_cut, "cut", A_GIMME, 0);
	
	
	// @method paste @digest Paste
	// @description Pastes the content contained in the global clipboard. <br />
	// If the clipboard contains a portion of score, this score is pasted at its exact original position. Two optional arguments
	// changes this behavior: the first argument is a floating point number setting the onset at which the copied portion should be pasted (use <b>()</b>
	// or <b>nil</b> to keep the original position, use the "end" symbol to paste at the end of the score,
    // use the "replace" symbol to paste replacing current selection), the second argument is an integer
    // setting the number of the uppermost voice for pasting (pasting will retain original voices if no second argument is set). <br />
    // If the clipboard contains duration line (breakpoints) content, and the "durationline" symbol is used as first argument,
    // the duration line is applied to all selected notes. <br />
	// If the clipboard contains slot content, and the "slot" symbol is used as first argument,
    // the slot content is applied to the selected notes. If an integer argument is given, and a single slot
	// was copied, the slot content is applied only to the specified slot number. This number can be replaced with the "active" symbol, and the 
	// slot content will be applied to the open slot window.
    // @marg 0 @name replace @optional 1 @type symbol
	// @marg 1 @name position_or_slot @optional 1 @type number/symbol
	// @marg 2 @name starting_voice_number @optional 1 @type int
    // @mattr repeat @type int @default 1 @digest Number of times pasting must be performed
    // @example paste @caption paste at original position
    // @example paste 1000 @caption paste at 1 sec
    // @example paste end @caption paste at the end (i.e. append)
    // @example paste end @repeat 200 @caption append 200 times
    // @example paste replace @caption paste replacing current selection
    // @example paste 1000 2 @caption paste at 1 sec starting at voice 2
    // @example paste () 2 @caption paste at original position starting at voice 2
    // @example paste slot @caption paste copied slot(s) to selection
    // @example paste slot all @caption the same
    // @example paste slot 3 @caption paste copied slot to slot 3 of selection
    // @example paste slot ampenv @caption paste copied slot to "ampenv" slot of selection
    // @example paste slot active @caption paste copied slot to currently open slot
    // @example paste durationline @caption paste copied duration line to selection
    // @seealso cut, copy
//	class_addmethod(c, (method) roll_paste, "paste", A_GIMME, 0);   // THIS MUST BE COMMENTED!!!!!
                                                                    // Paste must be indeed handled inside the anything method,
                                                                    // otherwise it treads over the external editor paste method


	// @method read @digest Open file
	// @description The <m>read</m> message will open any file that <o>bach.roll</o> is able to read. 
	// The file name (as symbol) can be given as optional first argument. If no such symbol is given, a dialog box will pop up
	// allowing the choice of the file to open.
	// Supported file types are: <br />
	// - Previously saved <o>bach.roll</o> content, either in native or in text format (see <m>write</m> and <m>writetxt</m> messages). 
	// These might include also files exported in bach format from OpenMusic or PWGL.<br />
	// - MIDI files. In this case, some message attributes are available to tailor importing. Available attributes are: <br />
	// * <b>tracks2voices</b>: if non-0, each track in the MIDI
	// file will be converted into a separate voice. <br />
	// * <b>chans2voices</b>: if non-0, each channel in
	// the MIDI file will be converted into a separate voice. <br />
	// * <b>markmeasures</b> and <b>markdivisions</b>: if non-0, a special "barline" or "division" marker will be added for each measure/division
	// in the MIDI file, according to the current time signature (this is especially recommended if you need to later quantize the <o>bach.roll</o> content) <br />
	// * <b>importbarlines</b> (default: 1): all the MIDI markers named "bach barline" are imported as barline markers.
	// In general, importbarlines and markmeasure should not be on at the same time. <br />
	// @marg 0 @name filename @optional 1 @type symbol
    // @mattr tracks2voices @type int @default 1 if the MIDI file format is 1, else 0 @digest For MIDI files, if non-zero, each MIDI track will be converted into a separate voice (default: )
    // @mattr chans2voices @type int @default 0 if the MIDI file format is 1, else 1 @digest For MIDI files, if non-zero, each MIDI channel will be converted into a separate voice
    // @mattr markmeasures @type int @default 1 @digest For MIDI files, if non-zero, adds barline markers
    // @mattr markdivisions @type int @default 0 @digest For MIDI files, if non-zero, adds divisions markers
    // @mattr importbarlines @type int @default 1 @digest For MIDI files, if non-zero MIDI markers named "bach barline" are imported as barline markers
    // @example read @caption open file via dialog box
    // @example read rollexample.txt @caption open specific bach text file
    // @example read goldberg3.mid @markmeasures 1 @caption open MIDI file and convert barlines to markers
    // @example read goldberg3.mid @markmeasures 1 @markdivisions 1 @caption also convert beats to markers
    // @seealso write, writetxt
	class_addmethod(c, (method) roll_read, "read", A_GIMME, 0);
    class_addmethod(c, (method) roll_read_singlesymbol, "readsinglesymbol", A_GIMME, 0); // only for drag'n'drop purposes
	
    
	
	// @method write @digest Save file in native format
	// @description The <m>write</m> message saves all the <o>bach.roll</o> content in native format (with extension .llll).
	// The file name (as symbol) can be given as optional first argument. If no such symbol is given, a dialog box will pop up
	// allowing the choice of the location and file name for saving.
	// @marg 0 @name filename @optional 1 @type symbol
    // @example write @caption save file in native format via dialog box
    // @example write myfile.llll @caption save bach file in native format
    // @seealso writetxt, exportmidi, read
	class_addmethod(c, (method) roll_write, "write", A_GIMME, 0);


	// @method writetxt @digest Save file in text format
	// @description The <m>writetxt</m> message saves all the <o>bach.roll</o> content, as llll, in text format (with extension .txt).
	// This will result in a readable text file, but it might also lead to some very slight loss of precision. This is usually negligible, but
	// if it bothers you, use the <m>write</m> message instead. <br />
    // @copy BACH_DOC_WRITETXT_TEXT_FORMAT_AND_ARGUMENTS
    // @example writetxt @caption export as a text file, opening a dialog box for the file name
    // @example writetxt myfile.txt @caption export as a text file with the provided file name
    // @example writetxt myfile.txt @maxdecimals 3 @caption export with a floating-point precision of 3 decimal digits
    // @example writetxt myfile.txt @maxdecimals 3 @wrap 40 @caption as the above, limiting the length of each line to 40 characters
    // @example writetxt myfile.txt @indent 1 @caption export indenting each sublist by one space per depth level
    // @example writetxt myfile.txt @indent 1 @maxdepth 2 @caption as the above, but only first-level sublists are placed on new lines
    // @example writetxt myfile.txt @maxdepth 1 @caption no indentation is performed
    // @marg 0 @name filename @optional 1 @type symbol
    // @mattr indent @type atom @default tab @digest Number of spaces for indentation or "tab" symbol
    // @mattr maxdepth @type int @default -1 @digest Maximum depth for new lines
    // @mattr wrap @type int @default 0 @digest Maximum number of characters per line (0 means: no wrapping)
    // @seealso write, exportmidi, read
	class_addmethod(c, (method) roll_writetxt, "writetxt", A_GIMME, 0);
	
	
    // @method exportmidi @digest Export as MIDI file
    // @description The <m>exportmidi</m> message saves the content of the <o>bach.roll</o> as a MIDI file.
    // The file name (as symbol) can be given as optional first argument. If no such symbol is given, a dialog box will pop up
    // allowing the choice of the location and file name for saving.
    // Furthermore, some exporting message attributes are available, and each has to be given as llll after the (optional) file name.
    // Available attributes are: <br />
    // - <b>exportmarkers</b> (default: 1): if non-0, all the markers in the score will be exported. <br />
    // - <b>voices</b> (default: <b>()</b>): if a list of voices is provided, then only the specified voices will be exported.
    // If no list is provided, then all the voices of the score will be exported. Ranges can also be
    // expressed, as sublists. For example, <b>(voices 1 3 5)</b> will export the first, third and fifth voice,
    // while <b>(voices (1 5) 8)</b> will export all the voices from 1 to 5, and the 8th voice. <br />
    // - <b>format</b> (default: 0): the MIDI file format (0 = single track, 1 = multiple tracks). <br />
    // – <b>resolution</b> (default: 960): the number of MIDI ticks per beat. <br />
    // - <b>exportbarlines</b> (default: 1): the barline markers are exported as MIDI marker events, with the name "bach barline". <br />
    // @marg 0 @name filename @optional 1 @type symbol
    // @mattr exportmarkers @type int @default 1 @digest If non-zero, exports all the markers
    // @mattr exportbarlines @type int @default 1 @digest Barline markers are exported as MIDI marker events, with the name "bach barline"
    // @mattr voices @type llll @default null @digest Numbers of voices to be exported (<b>null</b> means: all voices)
    // @mattr format @type int @default 0 @digest MIDI file format (0 = single track, 1 = multiple tracks)
    // @mattr resolution @type int @default 960 @digest Number of MIDI ticks per beat
    // @example exportmidi @caption export MIDI file via dialog box
    // @example exportmidi mymidi.mid @caption export MIDI file
    // @example exportmidi mymidi.mid (exportmarkers 0) @caption the same, but don't export markers
    // @example exportmidi mymidi.mid (voices 1 3) (format 1) @caption exports voices 1 and 3 in format 1
    // @example exportmidi mymidi.mid (voices ((1 3))) (format 1) @caption exports voices 1 through 3
    // @example exportmidi mymidi.mid (voices ((1 3) 4 7)) (format 1) @caption exports voices 1 through 3, 4 and 7
    // @example exportmidi mymidi.mid (resolution 1920) @caption exports with a resolution of 1920 ticks per beat
    // @seealso write, writetxt, read
    class_addmethod(c, (method) roll_exportmidi, "exportmidi", A_GIMME, 0);

    
	// @method exportom @digest Export to OpenMusic
	// @description The <m>exportom</m> message saves the content of the <o>bach.roll</o> object in a way that OpenMusic will be able to open
	// (by selecting "Import" and then "From bach" from the File menu). The content of a <o>bach.roll</o> can be imported into an OpenMusic's "chord-seq"
	// or "multi-seq".
	// The file name (as symbol) can be given as optional first argument. If no such symbol is given, a dialog box will pop up
	// allowing the choice of the location and file name for saving.
	// @marg 0 @name filename @optional 1 @type symbol
    // @example exportom forOM.txt @caption export file in OpenMusic format
    // @seealso write, writetxt, exportpwgl, exportmidi, read
	class_addmethod(c, (method) roll_exportom, "exportom", A_GIMME, 0);


	// @method exportpwgl @digest Export to PWGL
	// @description The <m>exportpwgl</m> message saves the content of the <o>bach.roll</o> object in a way that PWGL will be able to open
	// (by connecting the "import-bach" box, in the "pwgl2bach" library).
	// The file name (as symbol) can be given as optional first argument. If no such symbol is given, a dialog box will pop up
	// allowing the choice of the location and file name for saving.
	// @marg 0 @name filename @optional 1 @type symbol
    // @example exportpwgl @caption export file in PWGL format via dialog box
    // @example exportpwgl forPWGL.txt @caption export file in PWGL format
    // @seealso write, writetxt, exportom, exportmidi, read
	class_addmethod(c, (method) roll_exportpwgl, "exportpwgl", A_GIMME, 0);


	// @method resetslotinfo @digest Reset the slotinfo to the default one
	// @description @copy BACH_DOC_RESET_SLOTINFO
    // @seealso eraseslot
	class_addmethod(c, (method) roll_resetslotinfo, "resetslotinfo", 0);


    // @method resetslotinfo @digest Reset the custom articulations definition to the default one
    // @description @copy BACH_DOC_RESET_ARTICULATIONINFO
    class_addmethod(c, (method) roll_resetarticulationinfo, "resetarticulationinfo", 0);

    
    // @method resetslotinfo @digest Reset the custom notehead definition to the default one
    // @description @copy BACH_DOC_RESET_NOTEHEADINFO
    class_addmethod(c, (method) roll_resetnoteheadinfo, "resetnoteheadinfo", 0);

    
	// @method distribute @digest Evenly distribute onsets
	// @description Distributes onsets of selected items so that they are spaced evenly in time.
    // @seealso onset
	class_addmethod(c, (method) roll_distribute, "distribute", 0);

    
    // @method setnotationcolors @digest Change all notation colors at once
    // @description Set all the notation colors at once (accidentals, annotations, articulations, clefs, key signatures, lyrics,
    // main and auxiliary staff, notes, stems, rests, tempi), in RGBA format.
    // @marg 0 @name red @optional 0 @type float
    // @marg 1 @name green @optional 0 @type float
    // @marg 2 @name blue @optional 0 @type float
    // @marg 3 @name alpha @optional 0 @type float
    // @example setallnotationcolors 1. 0. 0. 1. @caption turn all notation red
    class_addmethod(c, (method) roll_anything, "setnotationcolors", A_GIMME, 0);

    

	// @method (mouse) @digest Edit content
	// @description @copy BACH_DOC_ROLL_EDIT_MOUSE_COMMANDS
	class_addmethod(c, (method) roll_mouseenter, "mouseenter", A_CANT, 0);
	class_addmethod(c, (method) roll_mouseleave, "mouseleave", A_CANT, 0);
	class_addmethod(c, (method) roll_mousemove, "mousemove", A_CANT, 0);
	class_addmethod(c, (method) roll_mousedown, "mousedown", A_CANT, 0);
	class_addmethod(c, (method) roll_mousedrag, "mousedrag", A_CANT, 0);
	class_addmethod(c, (method) roll_mouseup, "mouseup", A_CANT, 0);
	class_addmethod(c, (method) roll_mousewheel, "mousewheel", A_CANT, 0);
    class_addmethod(c, (method) roll_mousedoubleclick, "mousedoubleclick", A_CANT, 0);
	class_addmethod(c, (method) roll_oksize, "oksize", A_CANT, 0);
    
    
    // @method (drag) @digest Open file
    // @description Dragging a file on the object will load its content, if a proper readable format is recognized.
    class_addmethod(c, (method) roll_acceptsdrag_unlocked,	"acceptsdrag_unlocked", A_CANT, 0);
    class_addmethod(c, (method) roll_acceptsdrag_unlocked,	"acceptsdrag_locked", A_CANT, 0);



	// @method (keyboard) @digest Edit content
	// @description @copy BACH_DOC_ROLL_EDIT_KEYBOARD_COMMANDS
  	class_addmethod(c, (method) roll_key, "key", A_CANT, 0);
  	class_addmethod(c, (method) roll_keyfilter, "keyfilter", A_CANT, 0);
	class_addmethod(c, (method) roll_enter,	"enter", A_CANT, 0);
	class_addmethod(c, (method) roll_focusgained, "focusgained", A_CANT, 0);
	class_addmethod(c, (method) roll_focuslost, "focuslost", A_CANT, 0);
	class_addmethod(c, (method) roll_jsave, "jsave", A_CANT, 0);
	class_addmethod(c, (method) roll_edclose, "edclose", A_CANT, 0);
	class_addmethod(c, (method) roll_okclose, "okclose", A_CANT, 0);

	class_addmethod(c, (method) roll_inletinfo, "inletinfo", A_CANT, 0);
	class_addmethod(c, (method) roll_assist, "assist", A_CANT, 0);

	// @method undo @digest Perform an undo step
	// @description An <m>undo</m> message will perform an undo step.
    // @seealso redo, inhibitundo, droplastundo
	class_addmethod(c, (method) roll_undo, "undo", 0);

	// @method redo @digest Perform a redo step
	// @description A <m>redo</m> message will perform a redo step.
    // @seealso undo, inhibitundo, droplastundo
	class_addmethod(c, (method) roll_redo, "redo", 0);

	// @method inhibitundo @digest Temporarily disable/enable undo step creation
	// @description An <m>inhibitundo</m> message followed by any non-zero number will
	// temporarily disable any undo step creation. Use the same message followed by a zero
	// to re-enable undo steps.
    // @seealso droplastundo, undo, redo
	class_addmethod(c, (method) roll_inhibit_undo, "inhibitundo", A_LONG, 0);
	
	// @method droplastundo @digest Drop last undo step 
	// @description A <m>droplastundo</m> message will prune the last undo step, by merging
	// its information into the previous one.
    // @seealso inhibitundo, undo, redo
	class_addmethod(c, (method) roll_prune_last_undo_step, "droplastundo", 0);

    // @method checkdynamics @digest Check dynamics for selection
    // @description A <m>checkdynamics</m> message will check the correctness of the dynamics sequence, for the whole score or - if
    // "selection" is set as first argument, for the selected items only. The dynamics sequence considered is the one contained in the slot
    // linked to dynamics, if any (see <m>linkdynamicstoslot</m>), or in another slot, if given as integer argument. <br />
    // Message attributes are:
    // • "inconsistent": value is expected to be 0 or 1, telling if the checking should happen on inconsistent dynamics
    // • "unnecessary": value is expected to be 0 or 1, telling if the checking should happen on unnecessary dynamics
    // @marg 0 @name selection @optional 1 @type symbol
    // @marg 1 @name slot_number @optional 1 @type int
    // @mattr inconsistent @type int @default 1 @digest If non-zero, checks inconsistent dynamics
    // @mattr unnecessary @type int @default 1 @digest If non-zero, checks unnecessary dynamics
    // @seealso fixdynamics
    // @example checkdynamics @caption check dynamics throughout the whole score
    // @example checkdynamics selection @caption check dynamics, for selected items only
    // @example checkdynamics @inconsistent 0 @caption check only wrong dynamics, don't check repeated ones
    class_addmethod(c, (method) roll_anything, "checkdynamics", A_GIMME, 0);

    
    // @method fixdynamics @digest Fix dynamics for selection
    // @description A <m>fixdynamics</m> message will check the correctness of the dynamics sequence, for the whole score or - if
    // "selection" is set as first argument, for the selected items only. The dynamics sequence considered is the one contained in the slot
    // linked to dynamics, if any (see <m>linkdynamicstoslot</m>), or in another slot, if given as integer argument. <br />
    // If incorrect dynamics are found, they are also fixed.
    // Further message attributes are:
    // • "inconsistent": value is expected to be 0 or 1, telling if the fixing should happen on inconsistent dynamics
    // • "unnecessary": value is expected to be 0 or 1, telling if the fixing should happen on unnecessary dynamics
    // • "verbose": value is expected to be 0 or 1, telling if the fixing should log warnings in the max window for each found dynamics
    // @marg 0 @name selection @optional 1 @type symbol
    // @marg 1 @name slot_number @optional 1 @type int
    // @mattr inconsistent @type int @default 1 @digest If non-zero, fixes inconsistent dynamics
    // @mattr unnecessary @type int @default 1 @digest If non-zero, fixes unnecessary dynamics
    // @mattr verbose @type int @default 0 @digest If non-zero, logs warnings in the max window for each found dynamics
    // @seealso checkdynamics
    // @example fixdynamics @caption fix all dynamics issues
    // @example fixdynamics selection @caption same thing, for selected items only
    // @example fixdynamics @inconsistent 0 @caption fix only wrong dynamics, keep repeated ones
    // @example fixdynamics @unnecessary 0 @caption filter out dynamics repetitions
    class_addmethod(c, (method) roll_anything, "fixdynamics", A_GIMME, 0);

    
    // @method dynamics2velocities @digest Assign velocities depending on dynamics
    // @description A <m>dynamics2velocities</m> message will assign all the note velocities (and pitch breakpoint velocity,
    // if <m>breakpointshavevelocity</m> is non-zero), depending on the current dynamics and hairpins, for the whole score or - if
    // "selection" is set as first argument, for the selected items only. The dynamics sequence considered is the one contained in the slot
    // linked to dynamics, if any (see <m>linkdynamicstoslot</m>), or in another slot, if given as integer argument. <br />
    // Unless explicit dynamic-velocity mapping is defined, a default mapping is used. Two message attributes handle such default mapping: <br />
    // • the "maxchars" attribute sets the width of the dynamics spectrum; <br />
    // • the "exp" attribute sets the exponent for the conversion. <br />
    // @copy BACH_DOC_DYNAMICS_SPECTRUM
    // Moreover, the "mapping" attribute defines a non-standard or more general mapping via an llll specification. Such parameter has syntax
    // <b>(<m>dynamics1</m> <m>velocity1</m>) (<m>dynamics2</m> <m>velocity2</m>)</b>.
    // You might also define a the dynamics association you want to drift from the default values; any other marking which does not show up in the
    // mapping will be converted via the default conversion equation.
    // @marg 0 @name selection @optional 1 @type symbol
    // @marg 1 @name slot_number @optional 1 @type int
    // @mattr maxchars @type int @default 4 @digest Width of the dynamics spectrum
    // @mattr exp @type float @default 0.8 @digest Exponent for the conversion
    // @mattr mapping @type llll @digest Custom dynamics-to-velocity mapping via <b>(<m>dynamics</m> <m>velocity</m>)</b> pairs
    // @seealso velocities2dynamics, checkdynamics, fixdynamics
    // @example dynamics2velocities @caption convert dynamics to velocities throughout the whole score
    // @example dynamics2velocities selection @caption same thing, for selected items only
    // @example dynamics2velocities @mapping (mp 70) (mf 80) @caption customly map "mp" to velocity = 70 and "mf" to velocity = 80
    // @example dynamics2velocities @maxchars 6 @caption use broader dynamics spectrum (from "pppppp" to "ffffff")
    // @example dynamics2velocities @maxchars 2 @caption use narrower dynamics spectrum (from "pp" to "ff")
    // @example dynamics2velocities @maxchars 2 @exp 0.5 @caption use narrower spectrum and a steeper mapping curve
    // @example dynamics2velocities @exp 1. @caption use a linear mapping curve
    class_addmethod(c, (method) roll_anything, "dynamics2velocities", A_GIMME, 0);

    
    // @method velocities2dynamics @digest Infer dynamics depending on velocities
    // @description A <m>velocities2dynamics</m> message will infer and assign chord dynamics and hairpins, depending on the current velocities,
    // for the whole score or - if "selection" is set as first argument, for the selected items only.
    // The dynamics sequence considered is the one contained in the slot
    // linked to dynamics, if any (see <m>linkdynamicstoslot</m>), or in another slot, if given as integer argument. <br />
    // Unless explicit dynamic-velocity mapping is defined, a default mapping is used. Two message attributes handle such default mapping: <br />
    // • the "maxchars" attribute sets the width of the dynamics spectrum; <br />
    // • the "exp" attribute sets the exponent for the conversion. <br />
    // @copy BACH_DOC_DYNAMICS_SPECTRUM
    // Moreover, the "mapping" attribute defines a non-standard or more general mapping via an llll specification. Such parameter has syntax
    // <b>(mapping (<m>dynamics1</m> <m>velocity1</m>) (<m>dynamics2</m> <m>velocity2</m>)...)</b>.
    // Differently from <m>dynamics2velocities</m>, if you define a mapping, you need to define the velocity association for each of the dynamic marking
    // you want to use. <br />
    // An "unnecessary" attribute toggles whether unnecessary dynamic markings should by default be dropped (default is 1: yes, use 0 to turn this of). <br />
    // Finally, a "thresh" attribute sets a threshold for hairpin detection (default is 1., 0. meaning: no hairpin detection).
    // @marg 0 @name selection @optional 1 @type symbol
    // @marg 1 @name slot_number @optional 1 @type int
    // @mattr maxchars @type int @default 4 @digest Width of the dynamics spectrum
    // @mattr exp @type float @default 0.8 @digest Exponent for the conversion
    // @mattr mapping @type llll @digest Custom dynamics-to-velocity mapping via <b>(<m>dynamics</m> <m>velocity</m>)</b> pairs
    // @mattr unnecessary @type int @default 1 @digest If non-zero, drops unnecessary dynamic markings
    // @mattr thresh @type float @default 1. @digest Hairpin detection threshold
    // @seealso dynamics2velocities, checkdynamics, fixdynamics
    // @example velocities2dynamics @caption convert velocities to dynamics throughout the whole score
    // @example velocities2dynamics selection @caption same thing, for selected items only
    // @example velocities2dynamics @mapping (p 40) (mp 70) (mf 90) (f 110) @caption use only "p", "mp", "mf", "f" symbols, with the defined mappings
    // @example velocities2dynamics @maxchars 6 @caption use broader dynamics spectrum (from "pppppp" to "ffffff")
    // @example velocities2dynamics @maxchars 2 @caption use narrower dynamics spectrum (from "pp" to "ff")
    // @example velocities2dynamics @maxchars 2 @exp 0.5 @caption use narrower spectrum and a steeper mapping curve
    // @example velocities2dynamics @exp 1. @caption use a linear mapping curve
    // @example dynamics2velocities @thresh 10. @caption allow coarse hairpin detection
    // @example velocities2dynamics @thresh 0. @unnecessary 0 @caption assign a dynamic marking to each chord, no hairpins
    class_addmethod(c, (method) roll_anything, "velocities2dynamics", A_GIMME, 0);

    
	
	class_addmethod(c, (method) roll_getmaxID, "getmaxid", 0); // undocumented
	class_addmethod(c, (method) roll_notify, "bachnotify", A_CANT, 0);

	llllobj_class_add_out_attr(c, LLLL_OBJ_UI);


	notation_class_add_notation_attributes(c, k_NOTATION_OBJECT_ROLL);
	
	CLASS_ATTR_DEFAULT(c, "patching_rect", 0, "0 0 526 120"); // new dimensions // was: 134
	// @exclude bach.roll

	CLASS_STICKY_ATTR(c,"category",0,"Show");
    
	CLASS_ATTR_CHAR(c,"showstems",0, t_notation_obj, show_stems);
    CLASS_ATTR_STYLE_LABEL(c,"showstems",0,"enumindex","Show Stems");
    CLASS_ATTR_ENUMINDEX(c,"showstems", 0, "Don't Main Main And Auxiliary");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"showstems", 0, "2");
	CLASS_ATTR_ACCESSORS(c, "showstems", (method)NULL, (method)roll_setattr_showstems);
	CLASS_ATTR_BASIC(c,"showstems",0);
    // @description Toggles the display of the chord stems. Values are: 0 = don't show,
    // 1 = show main stem, 2 = show both main and auxiliary unison stems (default).

	CLASS_STICKY_ATTR_CLEAR(c, "category");

	
	CLASS_STICKY_ATTR(c,"category",0,"Font");

	CLASS_ATTR_SYM(c,"notationfont", 0, t_notation_obj, noteheads_font);
	CLASS_ATTR_ALIAS(c,"notationfont", "ntfont");
	CLASS_ATTR_STYLE_LABEL(c, "notationfont",0,"font","Notation Font");
	CLASS_ATTR_DEFAULTNAME_SAVE_PAINT(c,"notationfont",0,"\"November for bach\"");
	CLASS_ATTR_ACCESSORS(c, "notationfont", (method)NULL, (method)roll_setattr_noteheads_font);
	// @description @copy BACH_DOC_NOTATION_FONT

	CLASS_ATTR_SYM(c,"accidentalsfont", 0, t_notation_obj, accidentals_font);
	CLASS_ATTR_ALIAS(c,"accidentalsfont", "acfont");
	CLASS_ATTR_STYLE_LABEL(c, "accidentalsfont",0,"font","Accidentals Font"); 
	CLASS_ATTR_DEFAULTNAME_SAVE_PAINT(c,"accidentalsfont", 0, "\"November for bach\"");
	CLASS_ATTR_ACCESSORS(c, "accidentalsfont", (method)NULL, (method)roll_setattr_accidentals_font);
	// @description @copy BACH_DOC_ACCIDENTALS_FONT
    
    CLASS_ATTR_SYM(c,"articulationsfont", 0, t_notation_obj, articulations_font);
    CLASS_ATTR_ALIAS(c,"articulationsfont", "acfont");
    CLASS_ATTR_STYLE_LABEL(c, "articulationsfont", 0, "font", "Articulations Font");
    CLASS_ATTR_DEFAULTNAME_SAVE_PAINT(c,"articulationsfont", 0, "\"November for bach\"");
    CLASS_ATTR_ACCESSORS(c, "articulationsfont", (method)NULL, (method)roll_setattr_articulations_font);
    // @description @copy BACH_DOC_ARTICULATIONS_FONT

	CLASS_STICKY_ATTR_CLEAR(c, "category");

	
	CLASS_STICKY_ATTR(c,"category",0,"Settings");

    CLASS_ATTR_NOTATIONOBJ_SYMPTR(c, "clefs", 0, clefs_as_symlist, CONST_MAX_VOICES, roll_setattr_clefs);
	CLASS_ATTR_STYLE_LABEL(c,"clefs",0,"text","Clefs");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"clefs",0,"G");
	CLASS_ATTR_BASIC(c,"clefs",0); 
	// @description @copy BACH_DOC_CLEFS
	
	CLASS_ATTR_NOTATIONOBJ_SYMPTR(c, "keys", 0, keys_as_symlist, CONST_MAX_VOICES, roll_setattr_keys);
	CLASS_ATTR_STYLE_LABEL(c,"keys",0,"text","Keys");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"keys",0,"CM");
	CLASS_ATTR_BASIC(c,"keys",0);
	// @description @copy BACH_DOC_KEYS

	CLASS_ATTR_LLLL(c, "voicenames", 0, t_notation_obj, voicenames_as_llll, notation_obj_getattr_voicenames, notation_obj_setattr_voicenames);
	CLASS_ATTR_STYLE_LABEL(c,"voicenames",0,"text_large","Voice Names");
	CLASS_ATTR_SAVE(c, "voicenames", 0);
	CLASS_ATTR_PAINT(c, "voicenames", 0);
	// @description @copy BACH_DOC_VOICENAMES

	CLASS_ATTR_LLLL(c, "stafflines", 0, t_notation_obj, stafflines_as_llll, notation_obj_getattr_stafflines, notation_obj_setattr_stafflines);
	CLASS_ATTR_STYLE_LABEL(c,"stafflines",0,"text_large","Number Of Staff Lines");
	CLASS_ATTR_SAVE(c, "stafflines", 0);
	CLASS_ATTR_PAINT(c, "stafflines", 0);
	// @description @copy BACH_DOC_STAFFLINES

	CLASS_ATTR_LONG(c, "tonedivision", 0, t_notation_obj, tone_division); 
	CLASS_ATTR_STYLE_LABEL(c,"tonedivision",0,"text","Microtonal Division");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"tonedivision",0,"2");
	CLASS_ATTR_ACCESSORS(c, "tonedivision", (method)NULL, (method)roll_setattr_tonedivision);
	CLASS_ATTR_BASIC(c,"tonedivision",0);
	// @description @copy BACH_DOC_TONEDIVISION
	
	CLASS_ATTR_CHAR(c, "accidentalsgraphic", 0, t_notation_obj, accidentals_display_type); 
	CLASS_ATTR_STYLE_LABEL(c,"accidentalsgraphic",0,"enumindex","Accidental Graphic");
	CLASS_ATTR_ENUMINDEX(c,"accidentalsgraphic", 0, "None Classical Fraction Unreduced Fraction Cents");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"accidentalsgraphic", 0,"1");
	CLASS_ATTR_ACCESSORS(c, "accidentalsgraphic", (method)NULL, (method)roll_setattr_accidentalsgraphic);
	CLASS_ATTR_BASIC(c,"accidentalsgraphic",0);
	// @description @copy BACH_DOC_ACCIDENTALSGRAPHIC
	
	CLASS_ATTR_CHAR(c, "accidentalspreferences", 0, t_notation_obj, accidentals_preferences); 
	CLASS_ATTR_STYLE_LABEL(c,"accidentalspreferences",0,"enumindex","Accidental Preferences");
	CLASS_ATTR_ENUMINDEX(c,"accidentalspreferences", 0, "Auto Sharps Flats Custom");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"accidentalspreferences",0,"0");
	CLASS_ATTR_ACCESSORS(c, "accidentalspreferences", (method)NULL, (method)roll_setattr_accidentalspreferences);
	// @description @copy BACH_DOC_ACCIDENTALSPREFERENCES
	
	CLASS_ATTR_NOTATIONOBJ_SYMPTR(c, "enharmonictable", 0, full_acc_repr, CONST_MAX_VOICES, roll_setattr_enharmonictable);
	CLASS_ATTR_STYLE_LABEL(c,"enharmonictable",0,"text_large","Custom Enharmonic Table");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"enharmonictable",0,"default");
	// @description @copy BACH_DOC_ENHARMONICTABLE

	CLASS_ATTR_DOUBLE(c, "minlength", 0, t_notation_obj, minimum_length); 
	CLASS_ATTR_STYLE_LABEL(c,"minlength",0,"text","Minimum Length In Milliseconds");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"minlength", 0,"0");
	CLASS_ATTR_ACCESSORS(c, "minlength", (method)NULL, (method)roll_setattr_minlength);
	// @description Sets a minimum length, so that scores having less than the defined length will still be displayed with a scrollbar
	// until the defined minimum length.
	
	CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    
	CLASS_STICKY_ATTR(c,"category",0,"Appearance");

	CLASS_ATTR_NOTATIONOBJ_CHARPTR(c, "hidevoices", 0, hidevoices_as_charlist, CONST_MAX_VOICES, roll_setattr_hidevoices);
	CLASS_ATTR_STYLE_LABEL(c,"hidevoices",0,"text","Hide Voices");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"hidevoices",0,"0");
	// @description Decide which voices must be shown or hidden. A list of 0/1 integers is expected, one for each voice.
	// 0 means that the voice is visibile, 1 means that it is hidden.
	// If less symbols are entered, the other elements are considered to be 1.
	
	CLASS_ATTR_NOTATIONOBJ_DBLPTR(c, "voicespacing", 0, voiceuspacing_as_floatlist, CONST_MAX_VOICES + 1, roll_setattr_voicespacing);
	CLASS_ATTR_STYLE_LABEL(c,"voicespacing",0,"text","Voice Spacing");
	CLASS_ATTR_SAVE(c,"voicespacing",0);
	CLASS_ATTR_PAINT(c,"voicespacing",0);
	// @description @copy BACH_DOC_VOICESPACING

	CLASS_ATTR_DOUBLE(c,"zoom",0, t_notation_obj, horizontal_zoom);
	CLASS_ATTR_STYLE_LABEL(c,"zoom",0,"text","Horizontal Zoom %");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"zoom",0,"100.");
	CLASS_ATTR_ACCESSORS(c, "zoom", (method)NULL, (method)roll_setattr_zoom);
	CLASS_ATTR_BASIC(c,"zoom",0);
	// @description @copy BACH_DOC_ZOOM

	CLASS_ATTR_ATOM(c,"vzoom",0, t_notation_obj, vertical_zoom);
	CLASS_ATTR_STYLE_LABEL(c,"vzoom",0,"text","Vertical Zoom %");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"vzoom",0,"Auto");
	CLASS_ATTR_ACCESSORS(c, "vzoom", (method)NULL, (method)roll_setattr_vzoom);
	CLASS_ATTR_BASIC(c,"vzoom",0);
	// @description @copy BACH_DOC_VZOOM
	
	CLASS_ATTR_CHAR(c,"align",0, t_notation_obj, align_chords_with_what);
	CLASS_ATTR_STYLE_LABEL(c,"align",0,"enumindex","Chords Alignment Mode");
	CLASS_ATTR_ENUMINDEX(c,"align", 0, "Stems Notehead Center Notehead End");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"align",0,"1");
	// @description Sets the alignment mode for chords: <br />
	// - Stems: chords are aligned by their stem (whether or not it is displayed: see the <m>showstems</m> attribute). Two chords falling on the same time
	// instant will have their stems vertically aligned. <br />
	// - Notehead Center (default): means that chords are aligned by the center of the main notehead. This is similar to what happens in <o>bach.score</o>: 
	// two chords falling on the same time instant, one with stem up and one with stem down, will be aligned according to the position of their noteheads, and not 
	// the position of their stems. The noteheads will result aligned. <br />
	// - Notehead End: chords are aligned by the ending point of their noteheads. In this case, the duration line perfectly spans the time duration of the notes. 
	
	CLASS_ATTR_CHAR(c, "view", 0, t_notation_obj, view); 
	CLASS_ATTR_STYLE_LABEL(c,"view",0,"enumindex","View");
	CLASS_ATTR_ENUMINDEX(c,"view", 0, "Scroll Papyrus Page");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"view",0,"0");
	CLASS_ATTR_ACCESSORS(c, "view", (method)NULL, (method)roll_setattr_view);
	CLASS_ATTR_INVISIBLE(c, "view", ATTR_GET_OPAQUE | ATTR_SET_OPAQUE); // just for now
	// @exclude bach.roll
	
	CLASS_ATTR_LONG(c, "pagenumberoffset", 0, t_notation_obj, page_number_offset); 
	CLASS_ATTR_STYLE_LABEL(c,"pagenumberoffset",0,"text","Page Number Offset");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"pagenumberoffset",0,"0");
	CLASS_ATTR_INVISIBLE(c, "pagenumberoffset", ATTR_GET_OPAQUE | ATTR_SET_OPAQUE); // just for now
	// @exclude bach.roll
	
	CLASS_ATTR_CHAR(c,"showpagenumbers",0, t_notation_obj, show_page_numbers);
	CLASS_ATTR_STYLE_LABEL(c,"showpagenumbers",0,"onoff","Show Page Numbers");
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"showpagenumbers",0,"0");
	CLASS_ATTR_INVISIBLE(c, "showpagenumbers", ATTR_GET_OPAQUE | ATTR_SET_OPAQUE); // just for now
	// @exclude bach.roll

	CLASS_ATTR_CHAR(c,"customspacing",0, t_notation_obj, lambda_spacing);
	CLASS_ATTR_STYLE_LABEL(c,"customspacing",0,"enumindex","Custom Spacing Mode");
    CLASS_ATTR_ENUMINDEX(c,"customspacing", 0, "None Absolute Relative");
    CLASS_ATTR_ACCESSORS(c, "customspacing", (method)NULL, (method)roll_setattr_customspacing);
	CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"customspacing",0,"0");
    // @description Toggles the custom spacing mode. If active, this mode will send requests in the form
    // <b>timetopixel <m>ms</m></b> and <b>pixeltotime <m>pix</m></b> from the playout, which the user
    // should answer by re-injecting a response respectively in the form <b>pixel <m>pix</m></b> and
    // <b>time <m>ms</m></b> in the first inlet. This mechanisms allows the object to paint elements
    // according to the customly specified conversions. <br /> Three modes are available: <br />
    // None: no custom spacing (default). <br />
    // Absolute: pixels are considered inside the object box (0 being the leftmost line of the object box).
    // This solution will not automatically account for the scrollbar; this is especially handy if the spacing
    // needs to be retrieved from another notation object, such as a <o>bach.score</o>. <br />
    // Relative: pixels are considered from the standard 0 position onwards, and the user is not asked
    // to account for the scrollbar. <br />
    
	CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    
	CLASS_STICKY_ATTR(c,"category",0,"Play");

	CLASS_ATTR_LLLL(c, "loop", 0, t_notation_obj, loop_region_as_llll, roll_getattr_loop, roll_setattr_loop);
	CLASS_ATTR_STYLE_LABEL(c,"loop",0,"text_large","Loop Region");
	CLASS_ATTR_SAVE(c, "loop", 0);
	CLASS_ATTR_PAINT(c, "loop", 0);
	// @description Sets the loop region. Two numbers are expected: the starting and ending points of the loop region,
	// in milliseconds. You can substitute to the first number the "start" symbol to have the loop region start from the beginning (equivalent to 0).
	// You can substitute to the second number the "end" symbol to have the loop region last till the end of the <o>bach.roll</o>.
	// You can simply set a <b>loop all</b> to loop the entire <o>bach.roll</o>.

	CLASS_STICKY_ATTR_CLEAR(c, "category");

    
    
    CLASS_STICKY_ATTR(c,"category",0,"Notation");
    
    CLASS_ATTR_DOUBLE(c,"accidentaldecay",0, t_notation_obj, accidentals_decay_threshold_ms);
    CLASS_ATTR_STYLE_LABEL(c,"accidentaldecay",0,"text","Accidental Decay Time");
    CLASS_ATTR_DEFAULT_SAVE_PAINT(c,"accidentaldecay",0,"0");
    // @description Sets the decay time (in milliseconds) for non-classical accidental display, e.g. for naturalization of altered notes:
    // notes within the said threshold are naturalized, beyond this threshold, the naturalization decays, as if a new measure had started.
    // Leave 0 for "no decay" (accidentals are valid till the end of the roll, as if the whole roll were a single long measure).
    
    CLASS_STICKY_ATTR_CLEAR(c, "category");
    
    
	s_roll_class = c;
	class_register(CLASS_BOX, s_roll_class);

	dev_post("bach.roll compiled %s %s", __DATE__, __TIME__);
	return 0;
}

t_max_err roll_setattr_clefs(t_roll *x, t_object *attr, long ac, t_atom *av){
	t_max_err err = notation_obj_setattr_clefs((t_notation_obj *)x, attr, ac, av);
	recalculate_all_chord_parameters(x);
	return err;
}

t_max_err roll_setattr_keys(t_roll *x, t_object *attr, long ac, t_atom *av){
	t_max_err err = notation_obj_setattr_keys((t_notation_obj *)x, attr, ac, av);
	check_all_voices_fullaccpatterns((t_notation_obj *)x);
	recalculate_all_chord_parameters(x);
	return err;
}

t_max_err roll_setattr_tonedivision(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		long s = atom_getlong(av); 
		if (s<=2) s=2;
		if (x->r_ob.tone_division > s) {
			// are there userdefined accidentals?
			// if yes, we'll send a warning
			t_rollvoice *temp_vc;
			t_chord *chord;
			t_rollvoice *voice;
			t_chord *temp_ch; t_note *temp_nt;
			char there_are_user_accidentals = 0;
			for (temp_vc = x->firstvoice; temp_vc && (temp_vc->v_ob.number < x->r_ob.num_voices) && (!(there_are_user_accidentals)); temp_vc = temp_vc->next) 
				for (temp_ch = temp_vc->firstchord; temp_ch && (!(there_are_user_accidentals)); temp_ch = temp_ch->next) 
					for (temp_nt = temp_ch->firstnote; temp_nt && (!(there_are_user_accidentals)); temp_nt = temp_nt->next) 
						if (note_is_enharmonicity_userdefined(temp_nt))
							there_are_user_accidentals = 1;
			if (there_are_user_accidentals)
				object_warn((t_object *) x, "Warning: loosening tone division has made automatic accidentals of user-defined accidentals.");

			// going from a finer to a looser tonedivision : we change all the user-defined accidentals into automatic-accidentals
			voice = x->firstvoice;
			while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
				chord = voice->firstchord;
				while (chord){
					t_note *note = chord->firstnote;
					while (note) {
                        note_set_auto_enharmonicity(note);
						note = note->next;
					}
					chord = chord->next;
				} 
				voice = voice->next;
			}
		}
		x->r_ob.tone_division = s;
		if (x->r_ob.accidentals_display_type == k_ACCIDENTALS_CLASSICAL) { // classical graphic
			if ((x->r_ob.tone_division != 2) && (x->r_ob.tone_division != 4) && (x->r_ob.tone_division != 8))
				object_warn((t_object *)x, "bach.roll does not support graphical accidentals for the %d-tone division. Use fraction- or cents-representation instead.", s);
			else if (((x->r_ob.tone_division == 8) && (x->r_ob.accidentals_typo_preferences.binary_characters_depth < 8)) || 
					 ((x->r_ob.tone_division == 4) && (x->r_ob.accidentals_typo_preferences.binary_characters_depth < 4)) ||
					 ((x->r_ob.tone_division == 2) && (x->r_ob.accidentals_typo_preferences.binary_characters_depth < 2)))
				object_warn((t_object *)x, "The active accidental font does not support the %d-tone division. Use fraction- or cents-representation instead, or change font.", s);
		}

		object_attr_setdisabled((t_object *)x, gensym("accidentalspreferences"), x->r_ob.tone_division != 2 && x->r_ob.tone_division != 4 && x->r_ob.tone_division != 8);
		//object_attr_setdisabled((t_object *)x, gensym("enharmonictable"), x->r_ob.tone_division != 2 && x->r_ob.tone_division != 4 && x->r_ob.tone_division != 8);

		check_all_voices_fullaccpatterns((t_notation_obj *)x);
		recalculate_all_chord_parameters(x);
	}
	return MAX_ERR_NONE;
}

t_max_err roll_setattr_accidentalsgraphic(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		long s = atom_getlong(av); 
		if (s>4) s=1;
		x->r_ob.accidentals_display_type = s;
		if (s == 1) { // classical graphic
			if ((x->r_ob.tone_division != 2) && (x->r_ob.tone_division != 4) && (x->r_ob.tone_division != 8))
				object_warn((t_object *)x, "bach.roll does not support graphical accidentals for the %d-tone division. Use fraction- or cents-representation instead.", x->r_ob.tone_division);
			else if (((x->r_ob.tone_division == 8) && (x->r_ob.accidentals_typo_preferences.binary_characters_depth < 8)) || 
					 ((x->r_ob.tone_division == 4) && (x->r_ob.accidentals_typo_preferences.binary_characters_depth < 4)) ||
					 ((x->r_ob.tone_division == 2) && (x->r_ob.accidentals_typo_preferences.binary_characters_depth < 2)))
				object_warn((t_object *)x, "The active accidental font does not support the %d-tone division. Use fraction- or cents-representation instead, or change font.", x->r_ob.tone_division);
		}
		
		recalculate_all_chord_parameters(x);
	}
	return MAX_ERR_NONE;
}



t_max_err roll_setattr_minlength(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		x->r_ob.minimum_length = MAX(atom_getfloat(av), 0.); 
		if (!x->r_ob.creatingnewobj)
			recompute_total_length((t_notation_obj *) x);
	}
	return MAX_ERR_NONE;
}


t_max_err roll_setattr_customspacing(t_roll *x, t_object *attr, long ac, t_atom *av){
    if (ac && av) {
        x->r_ob.lambda_spacing = (e_custom_spacing_mode)CLAMP(atom_getlong(av), 0, 2);
        if (x->r_ob.lambda_spacing != k_CUSTOMSPACING_NONE && !x->r_ob.creatingnewobj) {
            recompute_total_length((t_notation_obj *) x);
            update_hscrollbar((t_notation_obj *)x, 0);
            invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
        }
    }
    return MAX_ERR_NONE;
}


t_max_err roll_setattr_accidentalspreferences(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		x->r_ob.accidentals_preferences = (e_accidentals_preferences) CLAMP(atom_getlong(av), 0, 3); 
		//object_attr_setdisabled((t_object *)x, gensym("enharmonictable"), x->r_ob.accidentals_preferences != k_ACC_CUSTOM);
		if (!x->r_ob.creatingnewobj)
			parse_fullaccpattern_to_voices((t_notation_obj *) x);
		recalculate_all_chord_parameters(x);
	}
	return MAX_ERR_NONE;
}

t_max_err roll_setattr_enharmonictable(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		long i;
		for (i = 0; i < ac && i < x->r_ob.num_voices; i++)
			x->r_ob.full_acc_repr[i] = atom_getsym(av+i);
		for (; i < x->r_ob.num_voices; i++) // repeat last one
			x->r_ob.full_acc_repr[i] = atom_getsym(av + ac - 1);
		if (!x->r_ob.creatingnewobj)
			parse_fullaccpattern_to_voices((t_notation_obj *) x);
		recalculate_all_chord_parameters(x);
	}
	return MAX_ERR_NONE;
}

void recalculate_all_chord_parameters(t_roll *x) {
	// recalculate all the chords parameters
	t_rollvoice *voice = x->firstvoice;
	t_chord *temp_ch;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		temp_ch = voice->firstchord;
		while (temp_ch) {
			temp_ch->need_recompute_parameters = true; // we have to recalculate chord parameters 
			temp_ch = temp_ch->next;
		}
		voice = voice->next;
	}
}

t_max_err roll_setattr_nonantialiasedstaff(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		x->r_ob.force_non_antialiased_staff_lines = atom_getlong(av);
		if (x->r_ob.force_non_antialiased_staff_lines) {
			adjust_zoom_for_non_antialiased_lines((t_notation_obj *)x);
			calculate_voice_offsets((t_notation_obj *) x);
		} else {
			x->r_ob.zoom_y = x->r_ob.zoom_y_with_antialias;
			x->r_ob.system_jump = get_system_jump((t_notation_obj *)x);
		}
	}
	return MAX_ERR_NONE;
}


t_max_err roll_setattr_showstems(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		x->r_ob.show_stems = (e_show_stems_preferences) atom_getlong(av);
		recalculate_all_chord_parameters(x);
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	}
	return MAX_ERR_NONE;
}



t_max_err roll_setattr_zoom(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		double z = atom_getfloat(av);
        double minzoom = CONST_MIN_ZOOM;
		change_zoom((t_notation_obj *) x, (z > minzoom) ? z : minzoom);
		recompute_total_length((t_notation_obj *)x);
		calculate_ms_on_a_line((t_notation_obj *) x);
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	}
	return MAX_ERR_NONE;
}

t_max_err roll_setattr_vzoom(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		if (ac && atom_gettype(av) == A_SYM && (atom_getsym(av) == _llllobj_sym_Auto || atom_getsym(av) == _llllobj_sym_auto))
			x->r_ob.link_vzoom_to_height = 1;
//		else if (x->r_ob.version_number < 6500) // previous version than the introduction of vzoom 
//			x->r_ob.link_vzoom_to_height = 1;
		else
			x->r_ob.link_vzoom_to_height = 0;
		
		if (x->r_ob.link_vzoom_to_height) { // AUTO vzoom
			t_object *pv; // the patcherview
			t_rect rect;
			atom_setsym(&x->r_ob.vertical_zoom, _llllobj_sym_Auto);
			pv = (t_object *)jpatcher_get_firstview(x->r_ob.patcher_parent);
			jbox_get_rect_for_view(&x->r_ob.j_box.l_box.b_ob, pv, &rect);
			notationobj_set_vzoom_depending_on_height((t_notation_obj *)x, rect.height);
		} else if (ac && is_atom_number(av)){ // MANUAL vzoom
			double z = MAX(1., atom_getfloat(av)); 
			atom_setfloat(&x->r_ob.vertical_zoom, z);
			x->r_ob.zoom_y = z/100.;
			x->r_ob.step_y = CONST_STEP_UY * x->r_ob.zoom_y;
			x->r_ob.zoom_y_with_antialias = x->r_ob.zoom_y;
			adjust_zoom_for_non_antialiased_lines((t_notation_obj *)x);
			x->r_ob.system_jump = get_system_jump((t_notation_obj *)x);
//		post("supposed: %f. this: %f.--> zoom %f", supposedheight, height, x->r_ob.zoom_y);
			calculate_voice_offsets((t_notation_obj *) x);
			recompute_total_length((t_notation_obj *) x);
            recalculate_all_chord_parameters(x);
			calculate_ms_on_a_line((t_notation_obj *) x);
			x->r_ob.needed_uheight = notationobj_get_supposed_standard_height((t_notation_obj *) x);
			redraw_vscrollbar((t_notation_obj *) x, 1); // redraw is inside here
		}
	}
	return MAX_ERR_NONE;
}

t_max_err roll_setattr_view(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		long z = atom_getlong(av); 
		if (z < 0) 
			x->r_ob.view = 0;
		else if (z > 2)
			x->r_ob.view = 2;
		else
			x->r_ob.view = z; 
		recompute_total_length((t_notation_obj *)x);
		recalculate_num_systems((t_notation_obj *) x);
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	}
	return MAX_ERR_NONE;
}

t_max_err roll_setattr_voicespacing(t_roll *x, t_object *attr, long ac, t_atom *av){
	return notation_obj_setattr_voicespacing((t_notation_obj *) x, attr, ac, av);
}

t_max_err roll_setattr_hidevoices(t_roll *x, t_object *attr, long ac, t_atom *av){
	return notation_obj_setattr_hidevoices((t_notation_obj *) x, attr, ac, av);
}

t_max_err roll_setattr_noteheads_font(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		long size = NULL;
		char *text = NULL;

		atom_gettext_debug(ac, av, &size, &text, OBEX_UTIL_ATOM_GETTEXT_SYM_NO_QUOTE);

		if (size && text) {
			x->r_ob.noteheads_font = gensym(text);
			load_notation_typo_preferences((t_notation_obj *) x, x->r_ob.noteheads_font);
            load_noteheads_typo_preferences((t_notation_obj *) x, x->r_ob.noteheads_font);
			recalculate_all_chord_parameters(x);
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
			bach_freeptr(text);
		}
	}
	return MAX_ERR_NONE;
}

t_max_err roll_setattr_accidentals_font(t_roll *x, t_object *attr, long ac, t_atom *av){
	if (ac && av) {
		long size = NULL;
		char *text = NULL;
		
		atom_gettext_debug(ac, av, &size, &text, OBEX_UTIL_ATOM_GETTEXT_SYM_NO_QUOTE);
		
		if (size && text) {
			x->r_ob.accidentals_font = gensym(text);
			load_accidentals_typo_preferences((t_notation_obj *) x, x->r_ob.accidentals_font);
			recalculate_all_chord_parameters(x);
			
			x->r_ob.key_signature_uwidth = get_max_key_uwidth((t_notation_obj *) x);
			calculate_ms_on_a_line((t_notation_obj *) x);
			recalculate_num_systems((t_notation_obj *)x);
			x->r_ob.system_jump = get_system_jump((t_notation_obj *)x);
			x->r_ob.firsttime = true;
			
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
			bach_freeptr(text);
		}
	}
	return MAX_ERR_NONE;
}

t_max_err roll_setattr_articulations_font(t_roll *x, t_object *attr, long ac, t_atom *av)
{
    if (ac && av) {
        long size = NULL;
        char *text = NULL;
        
        atom_gettext_debug(ac, av, &size, &text, OBEX_UTIL_ATOM_GETTEXT_SYM_NO_QUOTE);
        
        if (size && text) {
            t_symbol *font = gensym(text);
            x->r_ob.articulations_font = font;
            load_articulations_typo_preferences(&x->r_ob.articulations_typo_preferences, font);
            reset_all_articulations_positions((t_notation_obj *)x);
            x->r_ob.firsttime = true;
            
            invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
            bach_freeptr(text);
        }
    }
    return MAX_ERR_NONE;
}


void set_loop_region_from_llll(t_roll *x, t_llll* loop, char lock_mutex)
{
    if (lock_mutex)
        lock_general_mutex((t_notation_obj *)x);
    
	if (loop) {
		if (loop->l_head && hatom_gettype(&loop->l_head->l_hatom) == H_SYM && hatom_getsym(&loop->l_head->l_hatom) == _llllobj_sym_all) {
			x->r_ob.loop_region.start.position_ms = 0;
			x->r_ob.loop_region.end.position_ms = x->r_ob.length_ms_till_last_note;
	
		} else  {
			if (loop->l_head && is_hatom_number(&loop->l_head->l_hatom))
				x->r_ob.loop_region.start.position_ms = hatom_getdouble(&loop->l_head->l_hatom);
			if (loop->l_head && loop->l_head->l_next && is_hatom_number(&loop->l_head->l_next->l_hatom))
				x->r_ob.loop_region.end.position_ms = hatom_getdouble(&loop->l_head->l_next->l_hatom);
			
			if (x->r_ob.loop_region.end.position_ms < x->r_ob.loop_region.start.position_ms)
				swap_doubles(&x->r_ob.loop_region.start.position_ms, &x->r_ob.loop_region.end.position_ms);
		}
		
		llll_free(x->r_ob.loop_region_as_llll);
		x->r_ob.loop_region_as_llll = get_loop_region_as_llll((t_notation_obj *)x, false);
		
		if (x->r_ob.playing) {
			check_correct_scheduling((t_notation_obj *)x, false);
		}
	}
    
    if (lock_mutex)
        unlock_general_mutex((t_notation_obj *)x);
}


void set_loop_region_from_extremes(t_roll *x, double start, double end)
{
	t_llll *temp = llll_get();
	llll_appenddouble(temp, start, 0, WHITENULL_llll);
	llll_appenddouble(temp, end, 0, WHITENULL_llll);
	set_loop_region_from_llll(x, temp, true);
	recompute_total_length((t_notation_obj *)x);
	llll_free(temp);
}



t_max_err roll_setattr_loop(t_roll *x, t_object *attr, long ac, t_atom *av){
	t_llll *args = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, ac, av, LLLL_PARSE_RETAIN);
	set_loop_region_from_llll(x, args, true);
	llll_free(args);
	if (x->r_ob.notify_also_upon_messages && !x->r_ob.creatingnewobj)
		send_loop_region((t_notation_obj *)x, 6);
	return MAX_ERR_NONE;
}


void roll_assist(t_roll *x, void *b, long m, long a, char *s){
    if (m == ASSIST_INLET) { // @in 0 @type llll/bang @digest bang or llll containing gathered syntax of the entire object
		switch (a)			 // @description See the <m>bang</m> and <m>llll</m> methods for more information.
		{
			case 0:	
				sprintf(s, "llll: Entire Roll (or bang to Rebuild)");
				break;
			case 1:			// @in 1 @type llll @digest Onsets (in milliseconds) in separate syntax.
				sprintf(s, "llll: Onsets"); // @description See the <m>llll</m> method for more information.
				break;
			case 2:			// @in 2 @type llll @digest Pitches or MIDIcents in separate syntax.
				sprintf(s,  "llll: Pitches"); // @description See the <m>llll</m> method for more information.
				break;
			case 3:			// @in 3 @type llll @digest Durations (in milliseconds) in separate syntax
				sprintf(s, "llll: Durations"); // @description See the <m>llll</m> method for more information.
				break;
			case 4:			// @in 4 @type llll @digest Velocities (1 to 127) in separate syntax
				sprintf(s, "llll: Velocities"); // @description See the <m>llll</m> method for more information.
				break;
			case 5:			// @in 5 @type llll @digest Extras in separate syntax
				sprintf(s, "llll: Extras"); // @description See the <m>llll</m> method for more information.
				break;
		}
	}
    else {
		char *type = NULL;
		llllobj_get_llll_outlet_type_as_string((t_object *) x, LLLL_OBJ_UI, a, &type);
		switch (a)
		{
			case 0:	// @out 0 @type llll @digest Whole object dump
				sprintf(s, "llll (%s): Entire Roll", type); // @description The gathered syntax of the whole <o>bach.roll</o> (or a part of it, depending on the <m>dump</m> message arguments)
				break;										// is sent through this outlet. See the <m>dump</m> message to know more about the output syntax.
			case 1:	// @out 1 @type llll @digest Onsets		
				sprintf(s, "llll (%s): Onsets", type);	// @description The onsets (in milliseconds) in separate syntax.
				break;									// @copy BACH_DOC_ROLL_SEPARATE_SYNTAX
			case 2: // @out 2 @type llll @digest Pitches
				sprintf(s, "llll (%s): Pitches", type);	// @description The pitches or MIDIcents in separate syntax (see <m>outputpitchesseparate</m>).
				break;									// @copy BACH_DOC_ROLL_SEPARATE_SYNTAX
			case 3: // @out 3 @type llll @digest Durations
				sprintf(s, "llll (%s): Durations", type);	// @description The durations (in milliseconds) in separate syntax.
				break;										// @copy BACH_DOC_ROLL_SEPARATE_SYNTAX
			case 4:	// @out 4 @type llll @digest Velocities
				sprintf(s, "llll (%s): Velocities", type);	// @description The velocities (1 to 127) in separate syntax.
				break;										// @copy BACH_DOC_ROLL_SEPARATE_SYNTAX
			case 5:	// @out 5 @type llll @digest Extras										
				sprintf(s, "llll (%s): Extras", type);		// @description The extras in separate syntax.
				break;										// @copy BACH_DOC_SEPARATE_SYNTAX_EXTRAS
			case 6: // @out 6 @type llll @digest Playout
				sprintf(s, "llll (%s): Playout", type);		// @description Outlet sending information about elements being played, as well as query answers and notifications.
				break;										// @copy BACH_DOC_PLAYOUT_SYNTAX_ROLL
			case 7: // @out 7 @type bang @digest bang when changed
				sprintf(s, "bang When Changed");			// @description This outlet sends a bang whenever any element has been changed via the interface.
				break;
		}
	}
}

void roll_inletinfo(t_roll *x, void *b, long a, char *t)
{
	if (a)
		*t = 1;
}



void roll_adjustadditionalstartpad(t_roll *x){
	t_rollvoice *vc; t_chord *ch;
	double this_pad = 0;
	double this_stem_0 = onset_to_xposition((t_notation_obj *) x, 0, NULL);
	for (vc = x->firstvoice; vc && vc->v_ob.number < x->r_ob.num_voices; vc = vc->next)
		if (!vc->v_ob.hidden){
			for (ch = vc->firstchord; ch; ch = ch->next){
				double this_stem_x = onset_to_xposition((t_notation_obj *) x, ch->onset, NULL);
				double this_left_pos = this_stem_x - ch->left_uextension - this_stem_0;
				if (this_left_pos < this_pad) 
					this_pad = this_left_pos; 
			}
		}

	x->r_ob.additional_ux_start_pad = -this_pad;
	update_hscrollbar((t_notation_obj *) x, 0);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
}

void roll_fixvzoom(t_roll *x){
	t_atom av[1];
	atom_setfloat(av, x->r_ob.zoom_y * 100.);
	roll_setattr_vzoom(x, NULL, 1, av);
}



void roll_openslotwin(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	notation_obj_openslotwin((t_notation_obj *)x, s, argc, argv);
}



///// ACCIDENTALS HANDLING IN ROLL
void update_all_accidentals_for_voice_if_needed(t_roll *x, t_rollvoice *voice)
{
    switch (x->r_ob.show_accidentals_preferences) {
        case k_SHOW_ACC_ALL:
        case k_SHOW_ACC_NONE:
        case k_SHOW_ACC_CLASSICAL:
        case k_SHOW_ACC_ALLALTERED_NONATURALS:
            // nothing to do;
            return;
            
        default:
        {
            t_chord *chord;
            t_note *note;
            for (chord = voice->firstchord; chord; chord = chord->next) {
                char changed = false;
                for (note = chord->firstnote; note; note = note->next) {
                    note->show_accidental = true;
                    changed = true;
                }
                chord->need_recompute_parameters |= changed;
            }
        }
            break;
    }
}

void update_all_accidentals_for_chord_if_needed(t_roll *x, t_chord *chord)
{
    update_all_accidentals_for_voice_if_needed(x, chord->voiceparent);
}


void update_all_accidentals_if_needed(t_roll *x)
{
    t_rollvoice *voice;

    if (x->r_ob.show_accidentals_preferences == 0)
        return;
    
    for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next)
        update_all_accidentals_for_voice_if_needed(x, voice);
}



// QUERYING STUFF

void roll_dumpvoicepixelpos(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);
	send_voicepixelpos((t_notation_obj *) x, k_NOTATION_OBJECT_ROLL, x->r_ob.num_voices, x->firstvoice, 6, label);
}


// Deprecated; use the bottom one
void roll_gettimeatpixel(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	t_llll *outlist = llll_get();
	t_llll *input_as_llll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, s, argc, argv, LLLL_PARSE_CLONE);
	t_symbol *label = get_querying_label_from_querying_llll((t_notation_obj *) x, input_as_llll, true);
	double pixel = 0, ms = 0;
	
	if (input_as_llll && input_as_llll->l_head)
		llll_behead(input_as_llll);

	if (input_as_llll && input_as_llll->l_head && is_hatom_number(&input_as_llll->l_head->l_hatom))
		pixel = hatom_getdouble(&input_as_llll->l_head->l_hatom);
	
	ms = xposition_to_onset((t_notation_obj *) x, pixel, 0);
	
	llll_appendsym(outlist, _llllobj_sym_timeatpixel, 0, WHITENULL_llll);
	if (label)
		llll_appendsym(outlist, label, 0, WHITENULL_llll);
	llll_appenddouble(outlist, ms, 0, WHITENULL_llll);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
	llll_free(outlist);
	llll_free(input_as_llll);
}

void roll_pixeltotime(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    t_llll *outlist = llll_get();
    t_llll *input_as_llll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, s, argc, argv, LLLL_PARSE_CLONE);
    t_symbol *label = get_querying_label_from_querying_llll((t_notation_obj *) x, input_as_llll, true);
    double pixel = 0, ms = 0;
    
    if (input_as_llll && input_as_llll->l_head)
        llll_behead(input_as_llll);
    
    if (input_as_llll && input_as_llll->l_head && is_hatom_number(&input_as_llll->l_head->l_hatom))
        pixel = hatom_getdouble(&input_as_llll->l_head->l_hatom);
    
    ms = xposition_to_onset((t_notation_obj *) x, pixel, 0);
    
    llll_appendsym(outlist, _llllobj_sym_time, 0, WHITENULL_llll);
    if (label)
        llll_appendsym(outlist, label, 0, WHITENULL_llll);
    llll_appenddouble(outlist, ms, 0, WHITENULL_llll);
    llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
    llll_free(outlist);
    llll_free(input_as_llll);
}



void send_domain(t_roll *x, long outlet, t_symbol *label)
{
	t_llll *outlist = llll_get();
	llll_appendsym(outlist, _llllobj_sym_domain, 0, WHITENULL_llll);
	if (label)
		llll_appendsym(outlist, label, 0, WHITENULL_llll);
	if (x->r_ob.domain == DBL_MIN) { // not yet painted!!
		llll_appendsym(outlist, gensym("undefined"), 0, WHITENULL_llll);
	} else {
		llll_appenddouble(outlist, x->r_ob.screen_ms_start, 0, WHITENULL_llll);
		llll_appenddouble(outlist, x->r_ob.screen_ms_end, 0, WHITENULL_llll);
	}
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, outlet, outlist);
	llll_free(outlist);
}

void roll_getdomain(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	// outputting domain (in ms)
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);
	getdomain((t_notation_obj *) x);
	send_domain(x, 6, label);
}

void roll_getdomainpixels(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_llll *outlist = llll_get();
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);
	
	getdomain((t_notation_obj *) x);
	llll_appendsym(outlist, gensym("domainpixels"), 0, WHITENULL_llll);
	if (label)
		llll_appendsym(outlist, label, 0, WHITENULL_llll);
	llll_appenddouble(outlist, onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_start, NULL), 0, WHITENULL_llll);
	if (x->r_ob.view == k_VIEW_SCROLL)
		llll_appenddouble(outlist, onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_end, NULL), 0, WHITENULL_llll);
	else
		llll_appenddouble(outlist, onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_start + x->r_ob.ms_on_a_line, NULL), 0, WHITENULL_llll);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
	llll_free(outlist);
}

// deprecated, use the next one
void roll_getpixelpos(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_llll *outlist = llll_get();
	t_llll *input_as_llll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, s, argc, argv, LLLL_PARSE_CLONE);
	t_symbol *label = get_querying_label_from_querying_llll((t_notation_obj *) x, input_as_llll, true);
	double ms = 0;
	
	if (input_as_llll && input_as_llll->l_head)
		llll_behead(input_as_llll);

	if (input_as_llll && input_as_llll->l_head && is_hatom_number(&input_as_llll->l_head->l_hatom))
		ms = hatom_getdouble(&input_as_llll->l_head->l_hatom);

	llll_appendsym(outlist, gensym("pixelpos"), 0, WHITENULL_llll);
	if (label)
		llll_appendsym(outlist, label, 0, WHITENULL_llll);
	llll_appenddouble(outlist, onset_to_xposition((t_notation_obj *) x, ms, NULL), 0, WHITENULL_llll);

	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
	llll_free(outlist);
	llll_free(input_as_llll);
}


void roll_timetopixel(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
    t_llll *outlist = llll_get();
    t_llll *input_as_llll = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, s, argc, argv, LLLL_PARSE_CLONE);
    t_symbol *label = get_querying_label_from_querying_llll((t_notation_obj *) x, input_as_llll, true);
    double ms = 0;
    
    if (input_as_llll && input_as_llll->l_head)
        llll_behead(input_as_llll);
    
    if (input_as_llll && input_as_llll->l_head && is_hatom_number(&input_as_llll->l_head->l_hatom))
        ms = hatom_getdouble(&input_as_llll->l_head->l_hatom);
    
    llll_appendsym(outlist, _llllobj_sym_pixel, 0, WHITENULL_llll);
    if (label)
        llll_appendsym(outlist, label, 0, WHITENULL_llll);
    llll_appenddouble(outlist, onset_to_xposition((t_notation_obj *) x, ms, NULL), 0, WHITENULL_llll);
    
    llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
    llll_free(outlist);
    llll_free(input_as_llll);
}


void roll_dumpnotepixelpos(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);
	t_llll *outlist = get_pixel_values_as_llll(x);
	
	if (label)
		llll_prependsym(outlist, label, 0, WHITENULL_llll);		
    llll_prependsym(outlist, (s == gensym("dumppixelpos")) ? gensym("dumppixelpos") : gensym("notepixelpos"), 0, WHITENULL_llll);
	
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
	llll_free(outlist);
}

void roll_getvzoom(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);
	send_vzoom((t_notation_obj *) x, 6, label);
}

void roll_getzoom(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);
	send_zoom((t_notation_obj *) x, 6, label);
}

void roll_getlength(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	// outputting total length (in milliseconds)
	t_llll *outlist = llll_get();
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);

	recompute_total_length((t_notation_obj *)x);
	llll_appendsym(outlist, _llllobj_sym_length, 0, WHITENULL_llll);
	if (label)
		llll_appendsym(outlist, label, 0, WHITENULL_llll);
	llll_appenddouble(outlist, x->r_ob.length_ms_till_last_note, 0, WHITENULL_llll);
	
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
	llll_free(outlist);
}

void roll_getnumvoices(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	// outputting number of voices
	t_llll *outlist = llll_get();
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);

	llll_appendsym(outlist, gensym("numvoices"), 0, WHITENULL_llll);
	if (label)
		llll_appendsym(outlist, label, 0, WHITENULL_llll);
	llll_appendlong(outlist, x->r_ob.num_voices, 0, WHITENULL_llll);
	
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
	llll_free(outlist);
}

void roll_getnumchords(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	// outputting number of chords (for each voice)
	t_rollvoice *voice; 
	t_llll *outlist = llll_get();
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);

	llll_appendsym(outlist, gensym("numchords"), 0, WHITENULL_llll);
	if (label)
		llll_appendsym(outlist, label, 0, WHITENULL_llll);

	lock_general_mutex((t_notation_obj *)x);	
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
		llll_appendlong(outlist, voice->num_chords, 0, WHITENULL_llll);
	}
	unlock_general_mutex((t_notation_obj *)x);	
	
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
	llll_free(outlist);
}

void roll_getnumnotes(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	// outputting number of notes (for each voice, for each chord)
	t_rollvoice *voice; t_chord *chord;
	t_llll *outlist = llll_get();
	t_symbol *label = get_querying_label_from_GIMME((t_notation_obj *) x, s, argc, argv);
	
	llll_appendsym(outlist, gensym("numnotes"), 0, WHITENULL_llll);
	if (label)
		llll_appendsym(outlist, label, 0, WHITENULL_llll);

	lock_general_mutex((t_notation_obj *)x);	
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
		t_llll *voicelist = llll_get();
		for (chord = voice->firstchord; chord; chord = chord->next) 
			llll_appendlong(voicelist, chord->num_notes, 0, WHITENULL_llll);
		llll_appendllll(outlist, voicelist, 0, WHITENULL_llll);
	}
	unlock_general_mutex((t_notation_obj *)x);	
	
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 6, outlist);
	llll_free(outlist);
}


void roll_domain(t_roll *x, t_symbol *s, long argc, t_atom *argv){ 
	// change x_zoom, so that the domain is argv[0]
	if (argc==1) { // set the length
		double domain = atom_getfloat(argv);
		double this_domain;
		getdomain((t_notation_obj *) x);
		this_domain = x->r_ob.domain;
		if (domain>0){
			double old_zoom = x->r_ob.horizontal_zoom;
			change_zoom((t_notation_obj *) x, old_zoom * ((double) this_domain)/((double) domain));

			getdomain((t_notation_obj *) x);
			redraw_hscrollbar((t_notation_obj *)x, 1);
			if (x->r_ob.notify_also_upon_messages)
				send_domain(x, 6, NULL);
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
		} else {
			object_warn((t_object *)x, "Can't handle negative domains.");
		}
	} else if (argc >= 2) { // set the length
		double domain_start = atom_getfloat(argv);
		double domain_end = atom_getfloat(argv+1);
		double domain = domain_end - domain_start;
		double this_domain;

		if (argc >= 3)
			domain += xposition_to_onset((t_notation_obj *) x, atom_getfloat(argv+2) * x->r_ob.zoom_y, 0) - xposition_to_onset((t_notation_obj *) x, 0, 0); 

		getdomain((t_notation_obj *) x);
		
		this_domain = x->r_ob.domain;
		if (domain>0){
			double old_zoom = x->r_ob.horizontal_zoom;
			double new_zoom = old_zoom * ((double) this_domain)/((double) domain);
			change_zoom((t_notation_obj *) x, new_zoom);

			getdomain((t_notation_obj *) x);
			x->r_ob.hscrollbar_pos = ((double)domain_start)/(x->r_ob.length_ms - x->r_ob.domain);
			x->r_ob.screen_ms_start = domain_start;
			x->r_ob.screen_ms_end = domain_end; 
            x->r_ob.screen_ux_start = onset_to_unscaled_xposition((t_notation_obj *)x, x->r_ob.screen_ms_start);
            x->r_ob.screen_ux_end = onset_to_unscaled_xposition((t_notation_obj *)x, x->r_ob.screen_ms_end);
			redraw_hscrollbar((t_notation_obj *)x, 1);
			if (x->r_ob.notify_also_upon_messages)
				send_domain(x, 6, NULL);
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
		} else {
			object_warn((t_object *)x, "Can't handle negative domains.");
		}
	}
}

//deprecated:
void roll_setdomain(t_roll *x, t_symbol *s, long argc, t_atom *argv){ // TODO
/*	// change x_zoom, so that the domain is argv[0], but also preserve SHAPE of the roll (by changing onsets and durations
	if (argc==1) {
		long domain = atom_getlong(argv);
		getdomain((t_notation_obj *) x);
		long this_domain = x->r_ob.domain;
		if ((domain>0)&&(this_domain>0)){
			long i;
			float factor = ((float) this_domain)/((float) domain);
			x->r_ob.horizontal_zoom *= factor;
			for (i=0; i<x->r_ob.num_notes; i++) {
				x->r_ob.firstnotes[i].onset /= factor;
				x->r_ob.firstnotes[i].duration /= factor;
			}
            notationobj_redraw((t_notation_obj *) x);
		} else if (this_domain==0) {
			object_warn((t_object *)x, "The actual domain is 0: can't retain shape. Change it with the 'domain' message.");
		} else {
			object_warn((t_object *)x, "Can't handle negative domains.");
		}
	} else if (argc >= 2) {
		long domain_start = atom_getlong(argv);
		long domain_end = atom_getlong(argv+1);
		long domain = domain_end - domain_start;
		getdomain((t_notation_obj *) x);
		long this_domain = x->r_ob.domain;
		if ((domain>0)&&(this_domain>0)){
			long i;
			float factor = ((float) this_domain)/((float) domain);
			x->r_ob.horizontal_zoom *= factor;
			for (i=0; i<x->r_ob.num_notes; i++) {
				x->r_ob.firstnotes[i].onset /= factor;
				x->r_ob.firstnotes[i].duration /= factor;
			}
            notationobj_redraw((t_notation_obj *) x);
		} else if (this_domain==0) {
			object_warn((t_object *)x, "The actual domain is 0: can't retain shape. Change it with the 'domain' message.");
		} else {
			object_warn((t_object *)x, "Can't handle negative domains.");
		}
	}*/
}

void send_all_values_as_llll(t_roll *x, e_header_elems send_what) 
{
		send_extras_values_as_llll(x);
		send_velocities_values_as_llll(x);
		send_durations_values_as_llll(x);
		send_cents_values_as_llll(x);
		send_onsets_values_as_llll(x);
		send_roll_values_as_llll(x, send_what);
}

void roll_dump(t_roll *x, t_symbol *s, long argc, t_atom *argv){
	t_llll *headers;
	if (argc == 1 && (atom_gettype(argv) == A_SYM)) {
		t_symbol *sym = atom_getsym(argv);
		if ((sym == _llllobj_sym_onsets) || (sym == _llllobj_sym_onset)) {  
			send_onsets_values_as_llll(x);
			return;
		} else if ((sym == _llllobj_sym_cents) || (sym == _llllobj_sym_cent)) {
			send_cents_values_as_llll(x);
			return;
		} else if ((sym == _llllobj_sym_durations) || (sym == _llllobj_sym_duration)) {
			send_durations_values_as_llll(x);
			return;
		} else if ((sym == _llllobj_sym_velocities) || (sym == _llllobj_sym_velocity)) {
			send_velocities_values_as_llll(x);
			return;
		} else if ((sym == _llllobj_sym_extras) || (sym == _llllobj_sym_extra)) {
			send_extras_values_as_llll(x);
			return;
		} else if (sym == _llllobj_sym_separate) {
			send_extras_values_as_llll(x);
			send_velocities_values_as_llll(x);
			send_durations_values_as_llll(x);
			send_cents_values_as_llll(x);
			send_onsets_values_as_llll(x);
			return;
		} else if (sym == _llllobj_sym_roll) {
			send_roll_values_as_llll(x, k_HEADER_ALL); // dump full gathered syntax
			return;
		}
	} else if (argc == 0) {
		send_all_values_as_llll(x, k_HEADER_ALL); // dump all separate outlets and full gathered syntax
		return;
	}

	// otherwise, can have arguments:
	headers = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_RETAIN);
	send_roll_values_as_llll(x, header_objects_to_long(headers));
	if (headers) llll_free(headers);
}

void clear_all(t_roll *x) {
	long i; t_rollvoice *voice = x->firstvoice;
	lock_general_mutex((t_notation_obj *)x);
	for (i = 0; i < x->r_ob.num_voices; i++) { 
		if (voice)
			clear_voice(x, voice);
		voice = voice->next;
	}
	unlock_general_mutex((t_notation_obj *)x);
}

void clear_voice(t_roll *x, t_rollvoice *voice) {
	char need_check_scheduling = false;
	if (voice->lastchord) {
		t_chord *temp = voice->lastchord; t_chord *temp2;
		while (temp) {
			temp2 = temp;
			temp = temp->prev;
			if (delete_chord_from_voice((t_notation_obj *)x, temp2, NULL, false))
				need_check_scheduling = true;
		}
	}
	recompute_total_length((t_notation_obj *)x);
	if (need_check_scheduling)
		check_correct_scheduling((t_notation_obj *)x, false);
}

void roll_lambda(t_roll *x, t_symbol *s, long argc, t_atom *argv){ 
	if (argc && atom_gettype(argv) == A_SYM){
		t_symbol *router = atom_getsym(argv);
		if (router == _llllobj_sym_cents){
			roll_sel_change_cents(x, _llllobj_sym_lambda, argc - 1, argv + 1);
		} else if (router == _llllobj_sym_duration){
			roll_sel_change_duration(x, _llllobj_sym_lambda, argc - 1, argv + 1);
		} else if (router == _llllobj_sym_onset){
			roll_sel_change_onset(x, _llllobj_sym_lambda, argc - 1, argv + 1);
		} else if (router == _llllobj_sym_velocity){
			roll_sel_change_velocity(x, _llllobj_sym_lambda, argc - 1, argv + 1);
        } else if (router == _llllobj_sym_voice){
            roll_sel_change_voice(x, _llllobj_sym_lambda, argc - 1, argv + 1);
		} else if (router == _llllobj_sym_tail){
			roll_sel_change_tail(x, _llllobj_sym_lambda, argc - 1, argv + 1);
		} else if (router == _llllobj_sym_eraseslot){
			roll_sel_erase_slot(x, _llllobj_sym_lambda, argc - 1, argv + 1);
		} else if (router == _llllobj_sym_changeslotvalue){
			roll_sel_change_slot_value(x, _llllobj_sym_lambda, argc - 1, argv + 1);
		} else if (router == _llllobj_sym_addslot){
			roll_sel_add_slot(x, _llllobj_sym_lambda, argc - 1, argv + 1);
		} else if (router == _llllobj_sym_addbreakpoint){
			roll_sel_add_breakpoint(x, _llllobj_sym_lambda, argc - 1, argv + 1);
		} else if (router == _llllobj_sym_erasebreakpoints){
			roll_sel_erase_breakpoints(x, _llllobj_sym_lambda, 0, NULL);
        } else if (router == _llllobj_sym_name){
            notation_obj_name((t_notation_obj *)x, _llllobj_sym_lambda, argc - 1, argv + 1);
		}
	}
}


void roll_anything(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	
	long inlet = proxy_getinlet((t_object *) x);
	
	if (x->r_ob.is_sending_automessage) // automessage loop; we don't want it
		return;

	if (inlet == 0) {

		x->must_preselect_appended_chords = false;
        x->must_append_chords = false;
        x->must_apply_delta_onset = 0.;

/*        if (s == _llllobj_sym_addchords) {
			x->must_append_chords = true;
			if (argc == 0) {
				roll_bang(x);
				return;
			} else {
				s = NULL;
			}
		} else {
			x->must_append_chords = false;
            x->must_apply_delta_onset = 0.;
		} */

        if (true) { //(!x->must_append_chords || (x->must_append_chords && !s)) { // should be always
			t_llll *inputlist = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, s, argc, argv, LLLL_PARSE_CLONE); // We clone it: we operate destructively
			if (inputlist && (inputlist->l_size > 0)) {
				t_llllelem *firstelem = inputlist->l_head;
                char is_firstelem_symbol = (hatom_gettype(&firstelem->l_hatom) == H_SYM);

                if (is_firstelem_symbol && hatom_getsym(&firstelem->l_hatom) == _llllobj_sym_addchords) {
                    t_llllelem *el = firstelem->l_next;
                    x->must_append_chords = true;
                    if (!el) {
                        roll_bang(x);
                        llll_free(inputlist);
                        return;
                    } else {
                        if (el && is_hatom_number(&el->l_hatom)) {
                            x->must_apply_delta_onset = hatom_getdouble(&el->l_hatom);
                            llll_destroyelem(el);
                        }
                        hatom_setsym(&firstelem->l_hatom, _llllobj_sym_roll);
                    }
                }
                    
                if (hatom_gettype(&firstelem->l_hatom) == H_LLLL || (is_firstelem_symbol && hatom_getsym(&firstelem->l_hatom) == _llllobj_sym_roll)) {
                    create_whole_roll_undo_tick(x);
                    set_roll_from_llll(x, inputlist, true);
                    handle_rebuild_done((t_notation_obj *) x);
                    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_ROLL);
                
                } else if (is_firstelem_symbol) {
                    t_symbol *router = hatom_getsym(&firstelem->l_hatom);
                    
                    if (router == _sym_paste) {
                        roll_paste(x, s, argc, argv);
                    } else if (router == _sym_clear) {
                        create_whole_roll_undo_tick(x);
                        lock_general_mutex((t_notation_obj *)x);
                        if (inputlist->l_size > 1 && inputlist->l_head->l_next && is_hatom_number(&inputlist->l_head->l_next->l_hatom))
                            clear_roll_body(x, hatom_getlong(&inputlist->l_head->l_next->l_hatom) - 1);
                        else
                            roll_clear_all(x);
                        unlock_general_mutex((t_notation_obj *)x);
                        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CLEAR_ROLL);
                    } else if (router == _llllobj_sym_clearall) {
                        long i;
                        for (i = 1; i < 6; i++)
                            llllobj_store_llll((t_object *) x, LLLL_OBJ_UI, llll_get(), i);

/*                    } else if (router == gensym("inctest")) {
                        t_hatom *contained = &inputlist->l_head->l_next->l_hatom;
                        t_llll *container = hatom_getllll(&inputlist->l_head->l_next->l_next->l_hatom);
                        post("Contained? %ld", is_name_contained(contained, container));
*/
                    } else if (router == gensym("clearbreakpoints")) {
                        delete_all_breakpoints((t_notation_obj *)x, firstelem->l_next && is_hatom_number(&firstelem->l_next->l_hatom) ? hatom_getlong(&firstelem->l_next->l_hatom) - 1 : -1);
                        handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CLEAR_BREAKPOINTS);

                    } else if (router == gensym("explodechords")) {
                        roll_explodechords(x, is_symbol_in_llll_first_level(inputlist, _llllobj_sym_selection));

                    } else if (router == gensym("autospell")) {
                        notationobj_autospell_parseargs((t_notation_obj *)x, inputlist);

                        // custom spacing stuff
                    } else if (router == _llllobj_sym_pixel || router == _llllobj_sym_time ||
                               router == _llllobj_sym_timeatpixel || router == gensym("pixelpos")) {    // old, for compatibility
                        if (firstelem->l_next && is_hatom_number(&firstelem->l_next->l_hatom))
                            x->r_ob.lambda_val = hatom_getdouble(&firstelem->l_next->l_hatom);
                        
                    } else if (router == gensym("realtime")) {
                        if (firstelem->l_next && is_hatom_number(&firstelem->l_next->l_hatom))
                            notationobj_toggle_realtime_mode((t_notation_obj *)x, hatom_getlong(&firstelem->l_next->l_hatom));

                    } else if (router == gensym("setnotationcolors")) {
                        llll_destroyelem(firstelem);
                        notationobj_setnotationcolors((t_notation_obj *)x, inputlist);

                    } else if (router == _llllobj_sym_goto) {
                        llll_destroyelem(firstelem);
                        notationobj_goto_parseargs((t_notation_obj *)x, inputlist);
                        
                    } else if (router == gensym("checkdynamics")) {
                        char selection_only = false;
                        long inconsistent = true, unnecessary = true;
                        llll_destroyelem(firstelem);
                        long slot_num = x->r_ob.link_dynamics_to_slot - 1;
                        if (inputlist->l_head && hatom_getsym(&inputlist->l_head->l_hatom) == _llllobj_sym_selection) {
                            selection_only = true;
                            llll_behead(inputlist);
                        }
                        if (inputlist->l_head && is_hatom_number(&inputlist->l_head->l_hatom))
                            slot_num = hatom_getlong(&inputlist->l_head->l_hatom) - 1;
                        llll_parseargs_and_attrs((t_object *) x, inputlist, "ii", gensym("inconsistent"), &inconsistent, gensym("unnecessary"), &unnecessary);
                        if (slot_num >= 0 && slot_num < CONST_MAX_SLOTS)
                            notationobj_check_dynamics((t_notation_obj *)x, slot_num, inconsistent, unnecessary, false, false, selection_only, true);

                    } else if (router == gensym("fixdynamics")) {
                        char selection_only = false;
                        llll_destroyelem(firstelem);
                        long inconsistent = true, unnecessary = true;
                        long slot_num = x->r_ob.link_dynamics_to_slot - 1;
                        long fix_verbose = 0;
                        if (inputlist->l_head && hatom_getsym(&inputlist->l_head->l_hatom) == _llllobj_sym_selection) {
                            selection_only = true;
                            llll_behead(inputlist);
                        }
                        if (inputlist->l_head && is_hatom_number(&inputlist->l_head->l_hatom))
                            slot_num = hatom_getlong(&inputlist->l_head->l_hatom);
                        llll_parseargs_and_attrs((t_object *) x, inputlist, "iii", gensym("verbose"), &fix_verbose, gensym("inconsistent"), &inconsistent, gensym("unnecessary"), &unnecessary);
                        if (slot_num >= 0 && slot_num < CONST_MAX_SLOTS)
                            notationobj_check_dynamics((t_notation_obj *)x, slot_num, inconsistent, unnecessary, inconsistent, unnecessary, selection_only, fix_verbose);
                        
                        
                    } else if (router == gensym("dynamics2velocities")) {
                        char selection_only = false;
                        t_llll *mapping_ll = NULL;
                        llll_destroyelem(firstelem);
                        long slot_num = x->r_ob.link_dynamics_to_slot - 1;
                        double a_exp = CONST_DEFAULT_DYNAMICS_TO_VELOCITY_EXPONENT;
                        long maxchars = CONST_DEFAULT_DYNAMICS_SPECTRUM_WIDTH - 1;
                        if (inputlist->l_head && hatom_getsym(&inputlist->l_head->l_hatom) == _llllobj_sym_selection) {
                            selection_only = true;
                            llll_behead(inputlist);
                        }
                        if (inputlist->l_head && is_hatom_number(&inputlist->l_head->l_hatom)) {
                            slot_num = hatom_getlong(&inputlist->l_head->l_hatom) - 1;
                            llll_behead(inputlist);
                        }
                        llll_parseargs_and_attrs((t_object *)x, inputlist, "lid", gensym("mapping"), &mapping_ll, gensym("maxchars"), &maxchars, gensym("exp"), &a_exp);
                        if (slot_num >= 0 && slot_num < CONST_MAX_SLOTS)
                            notationobj_dynamics2velocities((t_notation_obj *)x, slot_num, mapping_ll, selection_only, MAX(0, maxchars + 1), CLAMP(a_exp, 0.001, 1.));
                        llll_free(mapping_ll);
                        
                    } else if (router == gensym("velocities2dynamics")) {
                        char selection_only = false;
                        t_llll *mapping_ll = NULL;
                        llll_destroyelem(firstelem);
                        long slot_num = x->r_ob.link_dynamics_to_slot - 1, delete_unnecessary = true;
                        double a_exp = CONST_DEFAULT_DYNAMICS_TO_VELOCITY_EXPONENT, approx_thresh = CONST_DEFAULT_VELOCITIES_TO_DYNAMICS_HAIRPIN_THRESH;
                        long maxchars = CONST_DEFAULT_DYNAMICS_SPECTRUM_WIDTH - 1;
                        if (inputlist->l_head && hatom_getsym(&inputlist->l_head->l_hatom) == _llllobj_sym_selection) {
                            selection_only = true;
                            llll_behead(inputlist);
                        }
                        if (inputlist->l_head && is_hatom_number(&inputlist->l_head->l_hatom)) {
                            slot_num = hatom_getlong(&inputlist->l_head->l_hatom) - 1;
                            llll_behead(inputlist);
                        }
                        llll_parseargs_and_attrs((t_object *)x, inputlist, "lidid", gensym("mapping"), &mapping_ll, gensym("maxchars"), &maxchars, gensym("exp"), &a_exp, gensym("unnecessary"), &delete_unnecessary, gensym("thresh"), &approx_thresh);
                        if (slot_num >= 0 && slot_num < CONST_MAX_SLOTS)
                            notationobj_velocities2dynamics((t_notation_obj *)x, slot_num, mapping_ll, selection_only, MAX(0, maxchars + 1), a_exp, delete_unnecessary, approx_thresh);
                        llll_free(mapping_ll);
                        
                        
                    } else if (router == gensym("refresh")) {
                        update_hscrollbar((t_notation_obj *)x, 1);
                        quick_notation_obj_recompute_all_chord_parameters((t_notation_obj *)x);
                        recompute_total_length((t_notation_obj *)x);
                        invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
                        
                    } else if (router == _llllobj_sym_interp && firstelem->l_next && is_hatom_number(&firstelem->l_next->l_hatom)) {
                        t_llll *out = notationobj_get_interp((t_notation_obj *)x, hatom_getdouble(&firstelem->l_next->l_hatom));
                        llll_prependsym(out, _llllobj_sym_interp, 0, WHITENULL_llll);
                        llllobj_outlet_llll((t_object *)x, LLLL_OBJ_UI, 6, out);
                        llll_free(out);
                        
                    } else if (router == _llllobj_sym_sample && firstelem->l_next && is_hatom_number(&firstelem->l_next->l_hatom)) {
                        t_llll *out;
                        if (llllelem_exists_and_is_sym(firstelem->l_next->l_next, gensym("ms")))
                            out = notationobj_get_sampling_ms((t_notation_obj *)x, hatom_getdouble(&firstelem->l_next->l_hatom));
                        else
                            out = notationobj_get_sampling((t_notation_obj *)x, hatom_getlong(&firstelem->l_next->l_hatom));
                        llll_prependsym(out, _llllobj_sym_sample, 0, WHITENULL_llll);
                        llllobj_outlet_llll((t_object *)x, LLLL_OBJ_UI, 6, out);
                        llll_check(out);
                        llll_free(out);
                        
                    } else if (router == _llllobj_sym_deletevoice) {
                        if (firstelem->l_next && is_hatom_number(&firstelem->l_next->l_hatom)) {
                            if (x->r_ob.num_voices == 1) {
                                object_error((t_object *)x, "Can't delete voices in an object having a single voice.");
                            } else {
                                long voicenum = hatom_getlong(&firstelem->l_next->l_hatom) - 1;
                                t_rollvoice *voice = nth_rollvoice(x, voicenum);
                                if (voice && voice->v_ob.number < x->r_ob.num_voices) {
                                    create_whole_roll_undo_tick(x);
                                    roll_delete_voice(x, voice);
                                    handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_DELETE_VOICE);
                                }
                            }
                        }
                        
                    } else if (router == _llllobj_sym_insertvoice) {
                        if (firstelem->l_next && is_hatom_number(&firstelem->l_next->l_hatom)) {
                            long voicenum = CLAMP(hatom_getlong(&firstelem->l_next->l_hatom) - 1, 0, x->r_ob.num_voices);
                            t_rollvoice *voice = nth_rollvoice(x, voicenum);
                            if (voice) {
                                t_llll *voice_content_ll = NULL;
                                char ref_def = true;
                                long ref_idx = (firstelem->l_next->l_next && hatom_gettype(&firstelem->l_next->l_next->l_hatom) == H_LONG) ? hatom_getlong(&firstelem->l_next->l_next->l_hatom) - 1 : -1;
                                if (ref_idx < 0 || ref_idx >= x->r_ob.num_voices) {
                                    ref_idx = (voice->prev ? voice->prev->v_ob.number : voice->v_ob.number);
                                    ref_def = false;
                                }
                                t_rollvoice *ref = nth_rollvoice(x, ref_idx);
                                
                                if (firstelem->l_next->l_next && hatom_gettype(&firstelem->l_next->l_next->l_hatom) == H_LLLL) {
                                    // Set voice content from llll
                                    voice_content_ll = llll_clone(hatom_getllll(&firstelem->l_next->l_next->l_hatom));
                                }
                                
                                create_whole_roll_undo_tick(x);
                                
                                roll_move_and_reinitialize_last_voice(x, voice->prev, x->r_ob.keys_as_symlist[ref_idx],
                                                                       ref->v_ob.clef, ref_def ? get_names_as_llll((t_notation_item *)ref, false) : llll_get(), ref->v_ob.midichannel, ref->v_ob.number + 1);
                                if (voice_content_ll) {
                                    long i;
                                    t_llll *ll = llll_get();
                                    llll_appendllll(ll, voice_content_ll);
                                    for (i = 0; i < voicenum; i++)
                                        llll_prependllll(ll, llll_get());
                                    x->must_append_chords = true;
                                    set_roll_from_llll(x, ll, true);
                                    x->must_append_chords = false;
                                    llll_free(ll);
                                }
                                
                                handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_INSERT_VOICE);
                            }
                        }
                        
                    } else if (router == _llllobj_sym_addchord || router == _llllobj_sym_gluechord) {
                        t_llllelem *secondelem, *chordinfo; //let's add a chord!
                        long voicenumber = 0;
                        llll_destroyelem(firstelem);
                        secondelem = inputlist->l_head;
                        if (secondelem && hatom_gettype(&secondelem->l_hatom) == H_LONG){
                            voicenumber = hatom_getlong(&secondelem->l_hatom) - 1; // 0-based inside, 1-based for the user
                            llll_destroyelem(secondelem);
                        }
                        chordinfo = inputlist->l_head;
                        if (chordinfo && hatom_gettype(&chordinfo->l_hatom) == H_LLLL) {
                            if (hatom_getsym(&firstelem->l_hatom) == _llllobj_sym_addchord) {
                                t_chord *newch = addchord_from_llll(x, hatom_getllll(&chordinfo->l_hatom), nth_rollvoice(x, voicenumber), true, true);
                                if (newch) {
                                    create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *) newch, k_UNDO_MODIFICATION_DELETE);
                                    handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_ADD_CHORD);
                                }
                            } else {
                                double threshold_ms = CONST_DEFAULT_GLUECHORD_THRESHOLD_MS;
                                double threshold_cents = CONST_DEFAULT_GLUECHORD_THRESHOLD_CENTS;
                                double smooth_ms = 0;
                                t_llllelem *elem = inputlist->l_head->l_next;
                                if (elem) {
                                    if (is_hatom_number(&elem->l_hatom))
                                        threshold_ms = fabs(hatom_getdouble(&elem->l_hatom));
                                    elem = elem->l_next;
                                    
                                    if (elem) {
                                        if (is_hatom_number(&elem->l_hatom))
                                            threshold_cents = fabs(hatom_getdouble(&elem->l_hatom));
                                        elem = elem->l_next;
                                        if (elem)
                                            smooth_ms = is_hatom_number(&elem->l_hatom) ? fabs(hatom_getdouble(&elem->l_hatom)) : -1;
                                    }
                                }
                                
                                gluechord_from_llll(x, hatom_getllll(&chordinfo->l_hatom), nth_rollvoice(x, voicenumber), threshold_ms, threshold_cents, smooth_ms);
                                handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_GLUE_CHORD);
                            }
                        }
                    } else if (!x->r_ob.itsme || router == _llllobj_sym_lambda) {
                        // send deparsed message to roll,
                        // TO DO: not the best way, though, at least for long messages!...
                        t_atom *av = NULL, rv;
                        long ac = llll_deparse(inputlist, &av, 0, 0);
                        if (ac) {
                            x->r_ob.itsme = true;
                            object_method_typed((t_object *)x, hatom_getsym(&firstelem->l_hatom), ac - 1, av + 1, &rv);
                            x->r_ob.itsme = false;
                        }
                        bach_freeptr(av);
                    } else {
                        post_unknown_message((t_object *) x, inputlist);
                    }
				} else {
					post_unknown_message((t_object *) x, inputlist);
				}
				llll_free(inputlist);
			}
		}

	// inlets > 0

	} else if (s == _sym_clear) {
		llllobj_store_llll((t_object *) x, LLLL_OBJ_UI, llll_get(), inlet);
	} else if (s == _sym_bang) {
		roll_bang(x);
	} else {
		llllobj_parse_and_store((t_object *) x, LLLL_OBJ_UI, s, argc, argv, inlet);
	}

}

void set_onsets_values_from_llll(t_roll *x, t_llll* onsets, char also_check_chords_order, long num_introduced_voices){
	if (onsets) {
		t_llllelem *elem;
		t_rollvoice *voice;
		t_llll *onsets_work = llll_clone(onsets); // we have to work on onsets (possibly wrapping them)
		
		// verify if we have to wrap
		elem = onsets_work->l_head;
		if (hatom_gettype(&elem->l_hatom) != H_LLLL)
			llll_wrap_once(&onsets_work);
		
		voice = x->firstvoice;
        for (elem = onsets_work->l_head; elem && voice->v_ob.number < num_introduced_voices; elem = (elem && elem->l_next) ? elem->l_next : elem) {
			if (voice) {
				long type = hatom_gettype(&elem->l_hatom);
				if (type == H_LLLL) {
					t_llll* voice_onsets = hatom_getllll(&elem->l_hatom); 
					set_voice_onsets_values_from_llll(x, voice_onsets, voice, also_check_chords_order);
				}
			} else 
				break;
			voice = voice->next;
		}
		llll_free(onsets_work);
	}
}

void set_durations_values_from_llll(t_roll *x, t_llll* durations, long num_introduced_voices){
	if (durations) {
		t_llllelem *elem; 

		t_rollvoice *voice = x->firstvoice;
		for (elem = durations->l_head; elem && voice->v_ob.number < num_introduced_voices; elem = (elem && elem->l_next) ? elem->l_next : elem) {
			if (voice) {
				long type = hatom_gettype(&elem->l_hatom);
				if (type == H_LLLL) {
					t_llll* voice_durations = hatom_getllll(&elem->l_hatom); 
					set_voice_durations_values_from_llll(x, voice_durations, voice);
				}
			} else 
				break;
			voice = voice->next;
		}
	}
}

void set_cents_values_from_llll(t_roll *x, t_llll* cents, char force_append_notes, long num_introduced_voices){
	if (cents) {
		t_llllelem *elem; 
		t_rollvoice *voice = x->firstvoice;
		for (elem = cents->l_head; elem && voice->v_ob.number < num_introduced_voices; elem = (elem && elem->l_next) ? elem->l_next : elem) {
			if (voice) {
				long type = hatom_gettype(&elem->l_hatom);
				if (type == H_LLLL) {
					t_llll* voice_llll = hatom_getllll(&elem->l_hatom); 
					set_voice_cents_values_from_llll(x, voice_llll, voice, force_append_notes);
				}
			} else 
				break;
			voice = voice->next;
		}
	}
}

void set_velocities_values_from_llll(t_roll *x, t_llll* velocities, long num_introduced_voices){
	if (velocities) {
		t_llllelem *elem;
		t_rollvoice *voice = x->firstvoice;
		for (elem = velocities->l_head; elem && voice->v_ob.number < num_introduced_voices; elem = (elem && elem->l_next) ? elem->l_next : elem) {
			if (voice) {
				long type = hatom_gettype(&elem->l_hatom);
				if (type == H_LLLL) {
					t_llll* voice_llll = hatom_getllll(&elem->l_hatom); 
					set_voice_velocities_values_from_llll(x, voice_llll, voice);
				}
			} else 
				break;
			voice = voice->next;
		}
	}
}

void set_graphic_values_from_llll(t_roll *x, t_llll* graphic){
	if (graphic) {
		t_llllelem *elem;
		t_rollvoice *voice = x->firstvoice;
		for (elem = graphic->l_head; elem; elem = elem->l_next) {
			if (voice) {
				long type = hatom_gettype(&elem->l_hatom);
				if (type == H_LLLL) {
					t_llll* voice_llll = hatom_getllll(&elem->l_hatom); 
					set_voice_graphic_values_from_llll(x, voice_llll, voice);
				}
			} else 
				break;
			voice = voice->next;
		}
	}
}

void set_breakpoints_values_from_llll(t_roll *x, t_llll* breakpoints){
	if (breakpoints) {
		t_llllelem *elem; 
		t_rollvoice *voice = x->firstvoice;
		for (elem = breakpoints->l_head; elem; elem = elem->l_next) {
			if (voice) { // cycle on the voices
				long type = hatom_gettype(&elem->l_hatom);
				if (type == H_LLLL) {
					t_llll* voice_llll = hatom_getllll(&elem->l_hatom); 
					set_voice_breakpoints_values_from_llll(x, voice_llll, voice);
				}
			} else 
				break;
			voice = voice->next;
		}
	}   
}

void set_slots_values_from_llll(t_roll *x, t_llll* slots){
	if (slots) {
		t_llllelem *elem;
		t_rollvoice *voice = x->firstvoice;
		for (elem = slots->l_head; elem; elem = elem->l_next) {
			if (voice) {
				long type = hatom_gettype(&elem->l_hatom);
				if (type == H_LLLL) {
					t_llll* voice_llll = hatom_getllll(&elem->l_hatom); 
					set_voice_slots_values_from_llll(x, voice_llll, voice);
				}
			} else 
				break;
			voice = voice->next;
		}
	}
}

void set_voice_onsets_values_from_llll(t_roll *x, t_llll* onsets, t_rollvoice *voice, char also_check_chords_order){
	t_llllelem *elem; t_chord *chord = voice->firstchord;
	for (elem = onsets->l_head; elem; elem = elem->l_next) {
		long type = hatom_gettype(&elem->l_hatom);
		if (type == H_LONG || type == H_DOUBLE || type == H_RAT) {
			double onset = hatom_getdouble(&elem->l_hatom); 
			if (chord) { // there's already a chord: we change its onset
				chord->onset = onset;
				chord = chord->next;
			} else { // there was NO chord: we create one
				t_note *this_nt;
				double argv[2]; 
				t_chord *newchord = NULL;
				argv[0] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
				argv[1] = CONST_DEFAULT_NEW_NOTE_CENTS;
				this_nt = build_note_from_ac_av((t_notation_obj *)x, 2, argv);
				newchord = addchord_from_notes(x, voice->v_ob.number, onset, -1, 1, this_nt, this_nt, true, 0);
				if (newchord)
					newchord->just_added_from_separate_parameters = true;
			}
		} else {
			if (chord)
				chord = chord->next;
		}
	}
	if (also_check_chords_order)
		check_chords_order_for_voice(x, voice); // USUALLY BAD THING TO DO, since user might insert things scrambled with matching parameters, so we usually will NOT want this especially upon reconstruction bang
	recompute_total_length((t_notation_obj *)x);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	verbose_print(x);
}

void set_voice_cents_values_from_llll(t_roll *x, t_llll* midicents, t_rollvoice *voice, char force_append_notes){
	t_llllelem *elem; t_chord *chord = voice->firstchord;
	for (elem = midicents->l_head; elem; elem = elem->l_next) { // x->firstvoice->firstchord
		long type = hatom_gettype(&elem->l_hatom);
		if (type == H_LLLL) { // there's a nested llll, so specifications for each note
			t_llll* note_midicents = hatom_getllll(&elem->l_hatom); 
			if (chord) { // there's already a chord: we change its cents
				if (note_midicents->l_size > 0) {
					t_llllelem *subelem; 
					t_note *note = chord->firstnote;
					for (subelem = note_midicents->l_head; subelem; subelem = subelem->l_next) {
						long subtype = hatom_gettype(&subelem->l_hatom);
						double cents; 
                        t_pitch pitch_in = t_pitch::NaP;
                        char set_pitch = 0;
						
						if ((subtype == H_LONG) || (subtype == H_DOUBLE) || (subtype == H_RAT)) {
							cents = hatom_getdouble(&subelem->l_hatom);
							modify_cents_if_nan_or_inf_and_warn((t_notation_obj *)x, &cents);
                            set_pitch = 1;
						} else if (subtype == H_PITCH) {
                            pitch_in = hatom_getpitch(&subelem->l_hatom);
                            cents = pitch_in.toMC();
                            set_pitch = 1;
						}
						
						if (set_pitch) {
							if (note) { // there's already a note: we change its cents
								note->midicents = cents;
								// note_compute_approximation((t_notation_obj *) x, note); // done by calculate_chord_parameters
                                note_set_enharmonicity(note, pitch_in);
								note = note->next;
							} else { // we create a note within the same chord!
								t_note *this_nt;
								double argv[2]; 
								argv[0] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
								argv[1] = cents;
								this_nt = build_note_from_ac_av((t_notation_obj *)x, 2, argv);
								if (force_append_notes)
									force_append_note((t_notation_obj *) x, chord, this_nt, 0);
								else
									insert_note((t_notation_obj *) x, chord, this_nt, 0);
                                
                                note_set_enharmonicity(this_nt, pitch_in);
							}
						}
					}
					chord->need_recompute_parameters = true; // we have to recalculate chord parameters 
				}
				chord = chord->next;
			} else { // there was NO chord: but we can create one
				t_chord *newchord = NULL; t_note *nt;
				double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
				long num_notes = note_midicents->l_size;
				double *argv = (double *) bach_newptr(2 * num_notes * sizeof(double)); 
				t_pitch *pitch_in = (t_pitch *) bach_newptr(num_notes * sizeof(t_pitch));
				long *screen_mc = (long *) bach_newptr(num_notes * sizeof(long)); 
				t_rational *screen_acc = (t_rational *) bach_newptr(num_notes * sizeof(t_rational)); 
				t_llllelem *subelem; 
				long i = 0, h = 0;
				for (subelem = note_midicents->l_head; subelem && i < 2 * num_notes - 1 && h < num_notes; subelem = subelem->l_next) {
					long subtype = hatom_gettype(&subelem->l_hatom);
					argv[i] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
					if ((subtype == H_LONG) || (subtype == H_DOUBLE) || (subtype == H_RAT)) {
						argv[i+1] = hatom_getdouble(&subelem->l_hatom);
						modify_cents_if_nan_or_inf_and_warn((t_notation_obj *)x, &(argv[i+1]));
                        pitch_in[h] = t_pitch::NaP;
					} else if (subtype == H_PITCH) {
                        pitch_in[h] = hatom_getpitch(&subelem->l_hatom);
                        argv[i+1] = pitch_in[h].toMC();
					} else {
                        pitch_in[h] = t_pitch::NaP;
						argv[i+1] = CONST_DEFAULT_NEW_NOTE_CENTS;
					}
					h++;
					i += 2;
				}
				newchord = addchord_from_values(x, voice->v_ob.number, num_notes, onset, -1, 2 * num_notes, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
				if (newchord) {
					newchord->just_added_from_separate_parameters = true;
					for (nt = newchord->firstnote, h = 0; nt && h < num_notes; nt = nt->next, h++)
                        note_set_enharmonicity(nt, pitch_in[h]);
				}
				bach_freeptr(argv);
				bach_freeptr(pitch_in);
				bach_freeptr(screen_mc);
				bach_freeptr(screen_acc);
			} 
			
		} else if ((type == H_LONG) || (type == H_DOUBLE) || (type == H_RAT) || (type == H_PITCH)) { // just one cent given: we apply it to the whole chord!!

			double cents; t_chord *newchord;
            char set_pitch = 0;
            t_pitch pitch_in = t_pitch::NaP;
			if ((type == H_LONG) || (type == H_DOUBLE) || (type == H_RAT)) {
				cents = hatom_getdouble(&elem->l_hatom); 
				set_pitch = 1;
			} else if (type == H_PITCH) {
                pitch_in = hatom_getpitch(&elem->l_hatom);
                cents = pitch_in.toMC();
				set_pitch = 1;
			}

			if (set_pitch) {
				if (chord) { // there's already a chord: we change all its cents
					t_note *note = chord->firstnote;
					while (note) {
						note->midicents = cents;
						// note_compute_approximation((t_notation_obj *) x, note); // done by calculate_chord_parameters
                        note_set_enharmonicity(note, pitch_in);
						note = note->next;
					}
					chord->need_recompute_parameters = true; // we have to recalculate chord parameters 
					chord = chord->next;
				} else { // there was NO chord: we create one, with just one note
					double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
					double argv[2]; 
					argv[0] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
					argv[1] = cents;
					newchord = addchord_from_values(x, voice->v_ob.number, 1, onset, -1, 2 , argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
					if (newchord) {
						newchord->just_added_from_separate_parameters = true;
                        if (newchord->firstnote)
                            note_set_enharmonicity(newchord->firstnote, pitch_in);
					}
				} 
			}
		}
	}
//	check_chords_order_for_voice(x, voice); // NO MORE NEEDED HERE
	recompute_total_length((t_notation_obj *)x);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	verbose_print(x);
}

void set_voice_durations_values_from_llll(t_roll *x, t_llll* durations, t_rollvoice *voice){
	t_llllelem *elem; 
	t_chord *chord = voice->firstchord;
	for (elem = durations->l_head; elem; elem = elem->l_next) {
		long type = hatom_gettype(&elem->l_hatom);
		if (type == H_LLLL) { // there's a nested llll, so specifications for each note
			t_llll* note_durations = hatom_getllll(&elem->l_hatom); 
			if (chord) { // there's already a chord: we change its cents
				if (note_durations->l_size > 0) {
					t_llllelem *subelem; 
					t_note *note = chord->firstnote;
					for (subelem = note_durations->l_head; subelem; subelem = subelem->l_next) {
						long subtype = hatom_gettype(&subelem->l_hatom);
						if (subtype == H_LONG || subtype == H_DOUBLE || subtype == H_RAT) {
							double duration = hatom_getdouble(&subelem->l_hatom);
							if (duration > 0.) {
								if (note){ // there's already a note: we change its duration
									note->duration = duration;
									note = note->next;
								} else { // we create a note within the same chord!
									t_note *this_nt;
									double argv[2]; 
									argv[0] = duration; 
									argv[1] = CONST_DEFAULT_NEW_NOTE_CENTS;
									this_nt = build_note_from_ac_av((t_notation_obj *)x, 2, argv);
									insert_note((t_notation_obj *) x, chord, this_nt, 0);
								}
							}
						}
					}
				}
				chord = chord->next;
			} else { // there was NO chord: but we can create one
				double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
				long num_notes = note_durations->l_size;
				double *argv = (double *) bach_newptr(2 * num_notes * sizeof(double)); 
				t_llllelem *subelem; long i = 0;
				t_chord *newchord = NULL;
				for (subelem = note_durations->l_head; subelem && i < 2 * num_notes - 1; subelem = subelem->l_next) {
					argv[i] = hatom_getdouble(&subelem->l_hatom); 
					argv[i+1] = CONST_DEFAULT_NEW_NOTE_CENTS;
					i += 2;
				}
				newchord = addchord_from_values(x, voice->v_ob.number, num_notes, onset, -1, 2 * num_notes, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
				if (newchord) {
					newchord->just_added_from_separate_parameters = true;
				}
				bach_freeptr(argv);
			} 
			
		} else if ((type == H_LONG) || (type == H_DOUBLE) || (type == H_RAT)) { // just one cent given: we apply it to the whole chord!!

			double duration = hatom_getdouble(&elem->l_hatom); 
			if (chord) { // there's already a chord: we change all its cents
				t_note *note = chord->firstnote;
				while (note) {
					note->duration = duration;
					note = note->next;
				}
				chord = chord->next;
			} else { // there was NO chord: we create one, with just one note
				double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
				double argv[2]; 
				t_chord *newchord;
				argv[0] = duration; 
				argv[1] = CONST_DEFAULT_NEW_NOTE_CENTS;
				newchord = addchord_from_values(x, voice->v_ob.number, 1, onset, -1, 2 , argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
				if (newchord) {
					newchord->just_added_from_separate_parameters = true;
				}
			} 
		}
	}
	
	// padding durations to all "just_added_from_separate_parameters" chords
	for (; chord && chord->just_added_from_separate_parameters; chord = chord->next) {
		t_note *nt;
		if (chord->prev && chord->prev->firstnote)
			for (nt = chord->firstnote; nt; nt = nt->next)
				nt->duration = chord->prev->firstnote->duration;
	}
			
	//	check_chords_order_for_voice(x, voice); // NO MORE NEEDED HERE
	recompute_total_length((t_notation_obj *)x);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	verbose_print(x);
}

void set_voice_velocities_values_from_llll(t_roll *x, t_llll* velocities, t_rollvoice *voice){
	t_llllelem *elem; t_chord *chord = voice->firstchord;
	for (elem = velocities->l_head; elem; elem = elem->l_next) {
		long type = hatom_gettype(&elem->l_hatom);
		if (type == H_LLLL) { // there's a nested llll, so specifications for each note
			t_llll* note_velocities = hatom_getllll(&elem->l_hatom); 
			if (chord) { // there's already a chord: we change its cents
				if (note_velocities->l_size > 0) {
					t_llllelem *subelem; t_note *note = chord->firstnote;
					for (subelem = note_velocities->l_head; subelem; subelem = subelem->l_next) {
						long subtype = hatom_gettype(&subelem->l_hatom);
						if ((subtype == H_LONG) || (subtype == H_DOUBLE) || (subtype == H_RAT)) {
							long velocity = CLAMP(hatom_getlong(&subelem->l_hatom), CONST_MIN_VELOCITY, CONST_MAX_VELOCITY);
							if (note){ // there's already a note: we change its cents
								note->velocity = velocity;
								note = note->next;
							} else { // we create a note within the same chord!
								t_note *this_nt;
								double argv[3]; 
								argv[0] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
								argv[1] = CONST_DEFAULT_NEW_NOTE_CENTS;
								argv[2] = velocity;
								this_nt = build_note_from_ac_av((t_notation_obj *)x, 3, argv);
								insert_note((t_notation_obj *) x, chord, this_nt, 0);
							}
						}
					}
				}
				chord = chord->next;
			} else { // there was NO chord: but we can create one
				double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
				long num_notes = note_velocities->l_size;
				double *argv = (double *) bach_newptr(3 * num_notes * sizeof(double)); 
				t_llllelem *subelem; long i = 0;
				t_chord *newchord = NULL;
				for (subelem = note_velocities->l_head; subelem  && i < 3 * num_notes - 2; subelem = subelem->l_next) {
					argv[i] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
					argv[i+1] = CONST_DEFAULT_NEW_NOTE_CENTS;
					argv[i+2] = hatom_getlong(&subelem->l_hatom);
					i += 3;
				}
				newchord = addchord_from_values(x, voice->v_ob.number, num_notes, onset, -1, 3 * num_notes, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
				if (newchord)
					newchord->just_added_from_separate_parameters = true;
				bach_freeptr(argv);
			} 
			
		} else if ((type == H_LONG) || (type == H_DOUBLE) || (type == H_RAT)) { // just one cent given: we apply it to the whole chord!!

			long velocity = CLAMP(hatom_getlong(&elem->l_hatom), CONST_MIN_VELOCITY, CONST_MAX_VELOCITY); 
			if (chord) { // there's already a chord: we change all its cents
				t_note *note = chord->firstnote;
				while (note) {
					note->velocity = velocity;
					note = note->next;
				}
				chord = chord->next;
			} else { // there was NO chord: we create one, with just one note
				double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
				double argv[3]; 
				t_chord *newchord = NULL;
				argv[0] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
				argv[1] = CONST_DEFAULT_NEW_NOTE_CENTS;
				argv[2] = velocity;
				newchord = addchord_from_values(x, voice->v_ob.number, 1, onset, -1, 3, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
				if (newchord)
					newchord->just_added_from_separate_parameters = true;
			} 
		}
	}

	// padding velocities to all "just_added_from_separate_parameters" chords
	for (; chord && chord->just_added_from_separate_parameters; chord = chord->next) {
		t_note *nt;
		if (chord->prev && chord->prev->firstnote)
			for (nt = chord->firstnote; nt; nt = nt->next)
				nt->velocity = chord->prev->firstnote->velocity;
	}

	//	check_chords_order_for_voice(x, voice); // NO MORE NEEDED HERE
	recompute_total_length((t_notation_obj *)x);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	verbose_print(x);
}

void set_extras_values_from_llll(t_roll *x, t_llll* extras){
	t_llllelem *elem;
	for (elem = extras->l_head; elem; elem = elem->l_next) {
		long type = hatom_gettype(&elem->l_hatom);
		if (type == H_SYM) { 
			// we just have 1 extra, so the user has omitted one level of ( .... )
			if (hatom_getsym(&elem->l_hatom) == _llllobj_sym_breakpoints) {
				llll_destroyelem(elem);
				set_breakpoints_values_from_llll(x, extras); 
			} else if (hatom_getsym(&elem->l_hatom) == _llllobj_sym_slots) {
				llll_destroyelem(elem);
				set_slots_values_from_llll(x, extras);
			} else if (hatom_getsym(&elem->l_hatom) == _llllobj_sym_graphic) {
				llll_destroyelem(elem);
				set_graphic_values_from_llll(x, extras);
			}
			return;
		} else if (type == H_LLLL) { // there's a nested llll, so specifications for each note
			t_llll* this_extra = hatom_getllll(&elem->l_hatom); 
			t_llllelem *pivot = this_extra->l_head;
			if (pivot) {
				if (hatom_gettype(&pivot->l_hatom) == H_SYM) {
					if (hatom_getsym(&pivot->l_hatom) == _llllobj_sym_breakpoints) {
						llll_destroyelem(pivot);
						set_breakpoints_values_from_llll(x, this_extra);
					} else if (hatom_getsym(&pivot->l_hatom) == _llllobj_sym_slots) {
						llll_destroyelem(pivot);
						set_slots_values_from_llll(x, this_extra);
					} else if (hatom_getsym(&pivot->l_hatom) == _llllobj_sym_graphic) {
						llll_destroyelem(pivot);
						set_graphic_values_from_llll(x, this_extra);
					}
				}
			}
		}

	}
	//	check_chords_order_for_voice(x, voice); // NO MORE NEEDED HERE
	recompute_total_length((t_notation_obj *)x);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	verbose_print(x);
}


void set_voice_graphic_values_from_llll(t_roll *x, t_llll* graphic, t_rollvoice *voice){
	t_llllelem *elem; t_chord *chord = voice->firstchord; // For instance: graphic = (((6000 1/4) (7000 1/8)) (7000 -1/4))
	for (elem = graphic->l_head; elem; elem = elem->l_next) { // elem cycles on the chords, e.g. elem = ((6000 1/4) (7000 1/8))    or    elem = (7000 -1/4) 
		long type = hatom_gettype(&elem->l_hatom);
		if (chord) { 
			
			if (type == H_LLLL) { // it has to be a LLLL
				t_llll *notes_graphic = hatom_getllll(&elem->l_hatom); 
				if (notes_graphic->l_size > 0) { // it has to be non nil!
					
					if (notes_graphic->l_depth == 2) { //there are specifications for each note, e.g.: elem = ((6000 1/4) (7000 1/8))
						
						t_llllelem *subelem; t_note *note = chord->firstnote;
						for (subelem = notes_graphic->l_head; subelem; subelem = subelem->l_next) { // subelem cycles on the notes
							long subtype = hatom_gettype(&subelem->l_hatom);
							if (subtype == H_LLLL) {
								t_llll *graphic = hatom_getllll(&subelem->l_hatom);
//								if ((graphic->l_size >= 2) && (graphic->l_depth == 1)) {
									if (note){ // there's already a note: we change its graphic values
										set_graphic_values_to_note_from_llll((t_notation_obj *) x, note, graphic);
										note = note->next;
									} else if ((graphic->l_size >= 2) && (graphic->l_depth == 1)) { // we create a note within the same chord!
										t_note *this_nt;
										double argv[2]; 
										long screen_mc = hatom_getlong(&graphic->l_head->l_hatom);
										t_rational screen_acc = hatom_getrational(&graphic->l_head->l_next->l_hatom);
										argv[0] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
										if (!(is_natural_note(screen_mc))) {
											object_warn((t_object *) x, "Wrong graphic approximation introduced. Dummy note created.");
											screen_mc = CONST_DEFAULT_NEW_NOTE_CENTS;
											screen_acc = long2rat(0);
										}
										argv[1] = rat2double(rat_long_sum(rat_long_prod(screen_acc, 200), screen_mc));
										this_nt = build_note_from_ac_av((t_notation_obj *)x, 2, argv);
										insert_note((t_notation_obj *) x, chord, this_nt, 0);
										set_graphic_values_to_note_from_llll((t_notation_obj *) x, this_nt, graphic);
									}
//								}
							}
						}
						
					} else if (notes_graphic->l_depth == 1) { // there is just 1 specification for the whole chord, e.g. elem = (7000 -1/4)
						
						if (notes_graphic->l_size >= 2) {
							t_note *note = chord->firstnote;
							while (note) {
								set_graphic_values_to_note_from_llll((t_notation_obj *) x, note, notes_graphic);
								note = note->next;
							}
						}
					}
					
					chord->need_recompute_parameters = true; // we have to recalculate chord parameters 
				}
			}
			
			chord = chord->next;
			
		} else { // there's no chord: we create one, but ONLY in backward compatibility mode. 
			
			if (type == H_LLLL) { // it has to be a LLLL
				t_llll *notes_graphic = hatom_getllll(&elem->l_hatom); 
				if (notes_graphic->l_size > 0) { // it has to be non nil!
					
                    if (x->r_ob.version_number < 70910) {
                        // OLD BW compatibility there are specifications for each note, e.g.: elem = ((6000 1/4) (7000 1/8))
                        if (notes_graphic->l_depth == 2) {
                            
                            double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
                            long num_notes = notes_graphic->l_size;
                            double *argv = (double *) bach_newptr(2 * num_notes * sizeof(double));
                            t_llllelem *subelem; long i = 0;
                            for (subelem = notes_graphic->l_head; subelem  && i < 2 * num_notes - 1; subelem = subelem->l_next) {
                                if (hatom_gettype(&subelem->l_hatom) == H_LLLL) {
                                    t_llll *subelemllll = hatom_getllll(&subelem->l_hatom);
                                    if (subelemllll->l_size >= 2) {
                                        long screen_mc = hatom_getlong(&subelemllll->l_head->l_hatom);
                                        t_rational screen_acc = hatom_getrational(&subelemllll->l_head->l_next->l_hatom);
                                        argv[i] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION;
                                        if (!(is_natural_note(screen_mc))) {
                                            object_warn((t_object *) x, "Wrong graphic approximation introduced. Dummy note created.");
                                            screen_mc = CONST_DEFAULT_NEW_NOTE_CENTS;
                                            screen_acc = long2rat(0);
                                        }
                                        argv[i+1] = rat2double(rat_long_sum(rat_long_prod(screen_acc, 200), screen_mc));
                                        i += 2;
                                    }
                                }
                            }
                            num_notes = i / 2;
                            if (num_notes > 0) {
                                t_chord *newchord = addchord_from_values(x, voice->v_ob.number, num_notes, onset, -1, 2 * num_notes, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
                                t_note *newnote = newchord->firstnote;
                                if (newnote) {
                                    for (subelem = notes_graphic->l_head; subelem; subelem = subelem->l_next) { // subelem cycles on the notes, e.g. subelem = (6000 1/4)
                                        t_llll *graphic = hatom_getllll(&subelem->l_hatom);
                                        set_graphic_values_to_note_from_llll((t_notation_obj *) x, newnote, graphic);
                                        newnote = newnote->next;
                                    }
                                }
                                newchord->need_recompute_parameters = true; // we have to recalculate chord parameters
                            }
                            bach_freeptr(argv);
                        } else if (notes_graphic->l_depth == 1) { // there's only one specification for the whole chord
                            if (notes_graphic->l_size >= 2) { // we need screen_cents and screen_accidental
                                t_chord *newchord;
                                double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
                                double argv[2];
                                long screen_mc = hatom_getlong(&notes_graphic->l_head->l_hatom);
                                t_rational screen_acc = hatom_getrational(&notes_graphic->l_head->l_next->l_hatom);
                                argv[0] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
                                if (!(is_natural_note(screen_mc))) {
                                    object_warn((t_object *) x, "Wrong graphic approximation introduced. Dummy note created.");
                                    screen_mc = CONST_DEFAULT_NEW_NOTE_CENTS;
                                    screen_acc = long2rat(0);
                                }
                                argv[1] = rat2double(rat_long_sum(rat_long_prod(screen_acc, 200), screen_mc));
                                newchord = addchord_from_values(x, voice->v_ob.number, 1, onset, -1, 2, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
                                set_graphic_values_to_note_from_llll((t_notation_obj *) x, newchord->firstnote, notes_graphic);
                                newchord->need_recompute_parameters = true; // we have to recalculate chord parameters 
                            }
                        }
                    }
					
				}
				
				
			} 
		}
	}
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	verbose_print(x);
}


void set_voice_breakpoints_values_from_llll(t_roll *x, t_llll* breakpoints, t_rollvoice *voice){
	t_llllelem *elem; t_chord *chord = voice->firstchord; // For instance: breakpoints = (  ((0 0 0) (1 1 1))   (((0 0 0) (1 1 1)) ((0 0 0) (0.5 20 0.) (1 1 1))) )
	for (elem = breakpoints->l_head; elem; elem = elem->l_next) { // elem cycles on the chords, e.g. elem = (((0 0 0) (1 1 1)) ((0 0 0) (0.5 20 0.) (1 1 1)))    or    elem = ((0 0 0) (1 1 1)) 
		long type = hatom_gettype(&elem->l_hatom);
		if (chord) { 
			
			if (type == H_LLLL) { // it must to be a LLLL
				t_llll *notes_bpt = hatom_getllll(&elem->l_hatom); 
				if (notes_bpt->l_size > 0) { // it has to be non nil!
					
					if (notes_bpt->l_depth == 3) { //there are specifications for each note, e.g.: elem = (((0 0 0) (1 1 1)) ((0 0 0) (0.5 20 0.) (1 1 1)))
						
						t_llllelem *subelem; t_note *note = chord->firstnote;
						for (subelem = notes_bpt->l_head; subelem; subelem = subelem->l_next) { // subelem cycles on the notes, e.g. subelem = ((0 0 0) (0.5 20 0.) (1 1 1))
							long subtype = hatom_gettype(&subelem->l_hatom);
							if (subtype == H_LLLL) { // it must be a LLLL
								t_llll *bpt = hatom_getllll(&subelem->l_hatom);
//								if ((bpt->l_size >= 1) && (bpt->l_depth == 2)) {
									if (note){ // there's already a note: we change its graphic values
										set_breakpoints_values_to_note_from_llll((t_notation_obj *) x, note, bpt);
										note = note->next;
									} else { // we create a note within the same chord!
										t_note *this_nt = build_default_note((t_notation_obj *) x);
										insert_note((t_notation_obj *) x, chord, this_nt, 0);
										set_breakpoints_values_to_note_from_llll((t_notation_obj *) x, this_nt, bpt);
									}
//								}
							}
						}
						
					} else if (notes_bpt->l_depth == 2) { // there is just 1 specification for the whole chord, e.g. elem = ((0 0 0) (1 1 1))
						
						if (notes_bpt->l_size >= 1) {
							t_note *note = chord->firstnote;
							while (note) {
								set_breakpoints_values_to_note_from_llll((t_notation_obj *) x, note, notes_bpt);
								note = note->next;
							}
						}
						
					}
					
					chord->need_recompute_parameters = true; // we have to recalculate chord parameters 
				}
			}
			
			chord = chord->next;
			
		} else { // there's no chord: we create one
			
			if (type == H_LLLL) { // it has to be a LLLL
				t_llll *notes_bpt = hatom_getllll(&elem->l_hatom); 
				if (notes_bpt->l_size > 0) { // it has to be non nil! it is the # of notes
					
					if (notes_bpt->l_depth == 3) { //there are specifications for each note,  e.g.: elem = (((0 0 0) (1 1 1)) ((0 0 0) (0.5 20 0.) (1 1 1)))
						
						double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
						long num_notes = notes_bpt->l_size;
						double *argv = (double *) bach_newptr(2 * num_notes * sizeof(double)); 
						t_llllelem *subelem; long i = 0;
						t_chord *newchord;
						t_note *newnote;
						for (subelem = notes_bpt->l_head; subelem && i < 2 * num_notes - 1; subelem = subelem->l_next) {
							if (hatom_gettype(&subelem->l_hatom) == H_LLLL) {
								t_llll *subelemllll = hatom_getllll(&subelem->l_hatom);
								if (subelemllll->l_size >= 1) {
									argv[i] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
									argv[i+1] = CONST_DEFAULT_NEW_NOTE_CENTS;
									i += 2;
								}
							}
						}
						num_notes = i / 2;
						newchord = addchord_from_values(x, voice->v_ob.number, num_notes, onset, -1, 2 * num_notes, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
						if (newchord) {
							newchord->just_added_from_separate_parameters = true;
							newnote = newchord->firstnote;
							for (subelem = notes_bpt->l_head; subelem; subelem = subelem->l_next) { // subelem cycles on the notes, e.g. subelem = ((0 0 0) (1 1 1))
								t_llll *bpts = hatom_getllll(&subelem->l_hatom);
								set_breakpoints_values_to_note_from_llll((t_notation_obj *) x, newnote, bpts);
								newnote = newnote->next;
							} 
							newchord->need_recompute_parameters = true; // we have to recalculate chord parameters 
						}
						bach_freeptr(argv);
						
					} else if (notes_bpt->l_depth == 2) { // there's only one specification for the whole chord, e.g. notes_bpt = ((0 0 0) (1 1 1))
						
						if ((notes_bpt->l_size >= 1) && (hatom_gettype(&notes_bpt->l_head->l_hatom) != H_LLLL)) { 
							double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
							double argv[2];
							t_chord *newchord;
							argv[0] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
							argv[1] = CONST_DEFAULT_NEW_NOTE_CENTS;
							newchord = addchord_from_values(x, voice->v_ob.number, 1, onset, -1, 2, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
							if (newchord) {
								newchord->just_added_from_separate_parameters = true;
								set_breakpoints_values_to_note_from_llll((t_notation_obj *) x, newchord->firstnote, notes_bpt);
								newchord->need_recompute_parameters = true; // we have to recalculate chord parameters 
							}
						}
					}
					
				}
				
				
			} 
		}
	}
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	verbose_print(x);
}


void set_voice_slots_values_from_llll(t_roll *x, t_llll* slots, t_rollvoice *voice){
	t_llllelem *elem; t_chord *chord = voice->firstchord; // For instance: slots = (  ((0 0 0) (1 1 1))   (((0 0 0) (1 1 1)) ((0 0 0) (0.5 20 0.) (1 1 1))) )
	for (elem = slots->l_head; elem; elem = elem->l_next) { // elem cycles on the chords, e.g. elem = (((0 0 0) (1 1 1)) ((0 0 0) (0.5 20 0.) (1 1 1)))    or    elem = ((0 0 0) (1 1 1)) 
		long type = hatom_gettype(&elem->l_hatom);
		if (chord) { 
			
			if (type == H_LLLL) { // it must to be a LLLL
				t_llll *notes_slots = hatom_getllll(&elem->l_hatom); 
				if (notes_slots ->l_size > 0) { // it has to be non nil!
					
					char specif_for_each_note = true; 
					if (hatom_gettype(&notes_slots->l_head->l_hatom) != H_LLLL) 
						specif_for_each_note = false;
					else if ((hatom_gettype(&notes_slots->l_head->l_hatom) == H_LLLL) &&
							 (hatom_getllll(&notes_slots->l_head->l_hatom)->l_size > 0) && 
							 (hatom_gettype(&hatom_getllll(&notes_slots->l_head->l_hatom)->l_head->l_hatom) != H_LLLL))
						specif_for_each_note = false;

					if (specif_for_each_note) { //there are specifications for each note, e.g.: elem = (((0 0 0) (1 1 1)) ((0 0 0) (0.5 20 0.) (1 1 1)))
						
						t_llllelem *subelem; t_note *note = chord->firstnote;
						for (subelem = notes_slots->l_head; subelem; subelem = subelem->l_next) { // subelem cycles on the notes, e.g. subelem = ((0 0 0) (0.5 20 0.) (1 1 1))
							long subtype = hatom_gettype(&subelem->l_hatom);
							if (subtype == H_LLLL) { // it must be a LLLL
								t_llll *slots = hatom_getllll(&subelem->l_hatom);
//								if (slots->l_size >= 1) {
									if (note){ // there's already a note: we change its graphic values
										set_slots_values_to_note_from_llll((t_notation_obj *) x, note, slots);
										note = note->next;
									} else { // we create a note within the same chord!
										t_note *this_nt = build_default_note((t_notation_obj *) x);
										insert_note((t_notation_obj *) x, chord, this_nt, 0);
										set_slots_values_to_note_from_llll((t_notation_obj *) x, this_nt, slots);
									}
//								}
							}
						}
						
					} else if (notes_slots->l_depth >= 2) { // there is just 1 specification for the whole chord, e.g. elem = ((0 0 0) (1 1 1))
						
						if (notes_slots->l_size >= 1) {
							t_note *note = chord->firstnote;
							while (note) {
								set_slots_values_to_note_from_llll((t_notation_obj *) x, note, notes_slots);
								note = note->next;
							}
						}
						
					}
					
					chord->need_recompute_parameters = true; // we have to recalculate chord parameters 
				}
			}
			
			chord = chord->next;
			
		} else { // there's no chord: we create one
			
			if (type == H_LLLL) { // it has to be a LLLL
				t_llll *notes_slots = hatom_getllll(&elem->l_hatom); 
				if (notes_slots->l_size > 0) { // it has to be non nil!
					
					if (hatom_gettype(&notes_slots->l_head->l_hatom) == H_LLLL) { //there are specifications for each note,  e.g.: elem = (((0 0 0) (1 1 1)) ((0 0 0) (0.5 20 0.) (1 1 1)))
						
						double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
						long num_notes = notes_slots->l_size;
						double *argv = (double *) bach_newptr(2 * num_notes * sizeof(double)); 
						t_llllelem *subelem; long z = 0;
						for (subelem = notes_slots->l_head; subelem && z < 2 * num_notes - 1; subelem = subelem->l_next) {
							if (hatom_gettype(&subelem->l_hatom) == H_LLLL) {
								t_llll *telem = hatom_getllll(&subelem->l_hatom);
								if (telem->l_size > 0) {
									argv[z] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
									argv[z+1] = CONST_DEFAULT_NEW_NOTE_CENTS;
									z += 2;
								}
							}
						}
						num_notes = z / 2;
						if (num_notes > 0) {
							t_chord *newchord = addchord_from_values(x, voice->v_ob.number,  num_notes, onset, -1, 2 * num_notes, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
							if (newchord) {
								t_note *newnote = newchord->firstnote;
								newchord->just_added_from_separate_parameters = true;
								for (subelem = notes_slots->l_head; subelem; subelem = subelem->l_next) { // subelem cycles on the notes, e.g. subelem = ((0 0 0) (1 1 1))
									//							t_llll *slots = llll_get();
									//							llll_appendllll(slots, hatom_getllll(&subelem->l_hatom), 0, WHITENULL_llll);
									set_slots_values_to_note_from_llll((t_notation_obj *) x, newnote, hatom_getllll(&subelem->l_hatom));
									//							set_slots_values_to_note_from_llll((t_notation_obj *) x, newnote, slots);
									newnote = newnote->next;
								} 
								newchord->need_recompute_parameters = true; // we have to recalculate chord parameters 
							}
						}
						bach_freeptr(argv);
					} else { // there's only one specification for the whole chord
						
						if (notes_slots->l_size >= 1) { 
							double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
							double argv[2];
							t_chord *newchord;
							t_llll *slots;
							argv[0] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
							argv[1] = CONST_DEFAULT_NEW_NOTE_CENTS;
							newchord = addchord_from_values(x, voice->v_ob.number, 1, onset, -1, 2, argv, NULL, NULL, 0, NULL, true, 0, NULL, false);
							if (newchord) {
								newchord->just_added_from_separate_parameters = true;
								slots = llll_get();
								llll_appendllll_clone(slots, notes_slots, 0, WHITENULL_llll, NULL);
								set_slots_values_to_note_from_llll((t_notation_obj *) x, newchord->firstnote, slots);
								newchord->need_recompute_parameters = true; // we have to recalculate chord parameters 
								llll_free(slots);
							}
						}
					}
				}
			} 
		}
	}
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	verbose_print(x);
}

// TODO: PROPERLY DEBUG NOTESLOTS 

/* 
void set_slots_values_from_llll(t_roll *x, t_llll* slots){
	t_llllelem *elem; t_chord *chord = x->firstchord;
	for (elem = slots->l_head; elem; elem = elem->l_next) { // elem iterates on the chords
		long type = hatom_gettype(&elem->l_hatom);
		if (type == H_LLLL) { // there's a nested llll, so specifications for each note
			t_llll *notes_slots = hatom_getllll(&elem->l_hatom); 
			if (chord) { 
				if (notes_slots->l_size > 0) {
					t_llllelem *subelem; t_note *note = chord->firstnote;
					for (subelem = notes_slots->l_head; subelem; subelem = subelem->l_next) { // subelem iterates on the notes
						long subtype = hatom_gettype(&subelem->l_hatom);
						if (subtype == H_LLLL) {
							t_llll *slots = hatom_getllll(&subelem->l_hatom);
							if (note) {
								set_slots_values_to_note_from_llll((t_notation_obj *) x, note, slots);
								note = note->next;
							}
							// else: NOTHING! if there's no note, we don't really want to add one just with graphic extras!!!
						}
					}
				}
				chord = chord->next;
			}  
			// else: NOTHING: if there's no chord, we don't really want to add one just with graphic extras!!!! 
		}
	}
    notationobj_redraw((t_notation_obj *) x);
	verbose_print(x);
} */

// beware: destructuve
t_chord *addchord_from_llll(t_roll *x, t_llll* chord, t_rollvoice* voice, char also_lock_general_mutex, char also_recompute_total_length) {
	t_chord *newchord = NULL;
	if (chord->l_size >= 2) { // AT LEAST onset + 1 note
		t_llllelem *secondelem = chord->l_head->l_next;
		unsigned long forced_chord_ID = notation_item_get_ID_from_llll(chord);
		
		if (hatom_gettype(&secondelem->l_hatom) == H_LLLL) {
			double onset = (chord && chord->l_head && is_hatom_number(&chord->l_head->l_hatom)) ? 
							hatom_getdouble(&chord->l_head->l_hatom) : ((voice->lastchord) ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.);
			long num_notes = get_num_llll_no_first_attribute_sym_in_llll(chord);
			double *argv = (double *) bach_newptr(2 * num_notes * sizeof(double));
			unsigned long *forced_note_IDs = (unsigned long *) bach_newptr(MAX(1, num_notes) * sizeof(unsigned long));
			t_llllelem *elem = chord->l_head;
			long i = 0;

			if (hatom_gettype(&elem->l_hatom) != H_LLLL)
				elem = get_next_llll_no_first_attribute_sym_elem(elem);

			for (i = 0; elem && i < num_notes; i++, elem = get_next_llll_no_first_attribute_sym_elem(elem)) {
				argv[2 * i] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
				argv[2 * i + 1] = CONST_DEFAULT_NEW_NOTE_CENTS;
				forced_note_IDs[i] = !elem ? 0 : notation_item_get_ID_from_llll(hatom_getllll(&elem->l_hatom));
			}
			
			if (also_lock_general_mutex)
				lock_general_mutex((t_notation_obj *)x);	
			
			newchord = addchord_from_values(x, voice->v_ob.number, num_notes, onset, -1, 2 * num_notes, argv, NULL, NULL, 0, NULL, false, forced_chord_ID, forced_note_IDs, false);
			if (newchord) {
				set_rollchord_values_from_llll((t_notation_obj *) x, newchord, chord, 0., true, also_recompute_total_length, true);
				newchord->need_recompute_parameters = true; 
			}
			
			if (also_lock_general_mutex)
				unlock_general_mutex((t_notation_obj *)x);

            if (newchord && is_solo_with_progeny((t_notation_obj *)x, (t_notation_item *) newchord))
                update_solos((t_notation_obj *)x);
            
			bach_freeptr(argv);
			bach_freeptr(forced_note_IDs);
		}
	}
	return newchord;
}

// beware!! this function changes the <chord> llll, destroying stuff inside it
void gluechord_from_llll(t_roll *x, t_llll* chord, t_rollvoice *voice, double threshold_ms, double threshold_cents, double smooth_ms){
	if (chord && chord->l_size >= 2) { // AT LEAST onset + 1 note
		t_llllelem *secondelem = chord->l_head->l_next;
		
		if (is_hatom_number(&chord->l_head->l_hatom) && hatom_gettype(&secondelem->l_hatom) == H_LLLL) {
			double onset = hatom_getdouble(&chord->l_head->l_hatom);
			
			// we check if we can "keep" some notes
			t_llllelem *note_elem = chord->l_head->l_next;

			while (note_elem) {
				t_llllelem *next_note_elem = note_elem->l_next;
				if (hatom_gettype(&note_elem->l_hatom) == H_LLLL){
					t_llll *note_llll = hatom_getllll(&note_elem->l_hatom);
					char first_element_type = hatom_gettype(&note_llll->l_head->l_hatom);
					if (note_llll->l_size >= 2 && (is_hatom_number(&note_llll->l_head->l_hatom) || first_element_type == H_SYM) 
						&& is_hatom_number(&note_llll->l_head->l_next->l_hatom)) {
						
						double cents = (first_element_type == H_SYM) ? 
							notename2midicents(x->r_ob.middleC_octave, &x->r_ob.last_used_octave, hatom_getsym(&note_llll->l_head->l_hatom)->s_name, NULL, NULL) :
							hatom_getdouble(&note_llll->l_head->l_hatom);
						double duration = hatom_getdouble(&note_llll->l_head->l_next->l_hatom);
						double velocity = note_llll->l_size >= 3 ? hatom_getlong(&note_llll->l_head->l_next->l_next->l_hatom) : -1;
						t_llll *breakpoints = find_sublist_with_router((t_notation_obj *)x, note_llll, _llllobj_sym_breakpoints);
						t_note *dummy_breakpoints_note = NULL;
						
                        if (cents > 4810 && cents < 4812) {
                            long foo = 7;
                            foo++;
                        }
                        
						if (breakpoints) { // only if the note to be inserted has breakpoints! We create a dummy note to more easily sample the breakpoints
							dummy_breakpoints_note = build_note((t_notation_obj *) x, cents, duration, velocity);
							set_breakpoints_values_to_note_from_llll((t_notation_obj *)x, dummy_breakpoints_note, breakpoints);
						}
						
						t_chord *tempch;
						t_note *tempnt, *foundnt = NULL;
						for (tempch = voice->firstchord; tempch; tempch = tempch->next){
							
							if (tempch->onset > onset + duration + threshold_ms)
								break;
							
							char chord_has_glissandi = false;
							for (tempnt = tempch->firstnote; tempnt; tempnt = tempnt->next) {
								if (tempnt->num_breakpoints > 2) {
									chord_has_glissandi = true;
									break;
								}
							}
							
							for (tempnt = tempch->firstnote; tempnt; tempnt = tempnt->next) {
								
								if (!chord_has_glissandi && tempnt->midicents > cents + threshold_cents)
									break;
								
								
								if (onset >= tempch->onset && onset <= tempch->onset + tempnt->duration + threshold_ms) {
									double left_mc = (tempnt->num_breakpoints <= 2 || tempnt->duration <= 0) ? tempnt->midicents : get_breakpoints_interpolated_mc((t_notation_obj *)x, tempnt, (onset - tempch->onset)/tempnt->duration, NULL);
									double right_mc = cents;
									if (fabs(left_mc - right_mc) <= threshold_cents) {
										foundnt = tempnt;
										break;
									}
								} else if (onset <= tempch->onset && onset + duration + threshold_ms >= tempch->onset){
									double left_mc = (!breakpoints || duration <= 0) ? cents : get_breakpoints_interpolated_mc((t_notation_obj *)x, dummy_breakpoints_note, (tempch->onset - onset)/duration, NULL);
									double right_mc = tempnt->midicents;
									if (fabs(left_mc - right_mc) <= threshold_cents) {
										// we might glue at left, but we can only do that if all chord notes can be leftly-glued to something!
										// otherwise we cannot move the chord onset at left!
										note_elem->l_thing.w_obj = tempnt; // we save in the l_thing the possible note for left-glueing
									}
								}
							}
							
							
						}
						
						if (foundnt){ // there's a note which we can make longer!
							if (onset + duration > foundnt->parent->onset + foundnt->duration) {
								
								// handling temporal slots' and breakpoints' information
								if (foundnt->duration <= 0) { // pathological case
									create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)foundnt->parent, k_UNDO_MODIFICATION_CHANGE);
									set_slots_values_to_note_from_llll((t_notation_obj *)x, foundnt, find_sublist_with_router((t_notation_obj *)x, note_llll, _llllobj_sym_slots));
									set_breakpoints_values_to_note_from_llll((t_notation_obj *)x, foundnt, find_sublist_with_router((t_notation_obj *)x, note_llll, _llllobj_sym_breakpoints));
								} else if (duration > 0){
									double start_glued_note_portion_rel_x = CLAMP((foundnt->parent->onset + foundnt->duration - onset)/duration, 0., 1.);
									create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)foundnt->parent, k_UNDO_MODIFICATION_CHANGE);
									glue_portion_of_temporal_slots((t_notation_obj *)x, foundnt, note_llll, start_glued_note_portion_rel_x, 1., (onset + duration - (foundnt->parent->onset + foundnt->duration))/foundnt->duration, 1, smooth_ms);
									glue_portion_of_breakpoints((t_notation_obj *)x, foundnt, note_llll, dummy_breakpoints_note, start_glued_note_portion_rel_x, 1., (onset + duration - (foundnt->parent->onset + foundnt->duration))/foundnt->duration, 1, smooth_ms);
								}
								
								// updating velocity 
								if (velocity > 0)
									foundnt->velocity = CLAMP(((foundnt->velocity * foundnt->duration) + (velocity * duration))/(foundnt->duration + duration), CONST_MIN_VELOCITY, CONST_MAX_VELOCITY);
								
								// updating duration
								foundnt->duration = onset + duration - foundnt->parent->onset; //update duration
 							}
							llll_destroyelem(note_elem); // delete the note from the chord llll
                        } else {
                            char foo = 7;
                            foo++;
                        }
						
						if (breakpoints) 
							free_note((t_notation_obj *)x, dummy_breakpoints_note);
					}
				}
				note_elem = next_note_elem;
			}
			
			
			// see if we can left-glue some notes
			note_elem = chord->l_head->l_next;
			while (note_elem) {
				t_llllelem *next = note_elem->l_next;
				
				if (note_elem->l_thing.w_obj != NULL) {
					t_note *gluenote = (t_note *)note_elem->l_thing.w_obj;
					t_chord *parent = gluenote->parent;
					t_note *temp; 
					t_llllelem *temp_el;
					
					// are all note of parent chord left-gluable to something? 
					char all_left_gluable = true;
					for (temp = parent->firstnote; temp; temp = temp->next) {
						char found = false;
						for (temp_el = chord->l_head->l_next; temp_el; temp_el = temp_el->l_next) {
							if ((t_note *)temp_el->l_thing.w_obj == temp) {
								found = true;
								break;
							}
						}
						if (!found)
							all_left_gluable = false;
					}
					
					if (all_left_gluable) {
						create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)parent, k_UNDO_MODIFICATION_CHANGE);
						for (temp = parent->firstnote; temp; temp = temp->next) {
							temp_el = chord->l_head->l_next;
							while (temp_el) {
								t_llll *note_ll = hatom_getllll(&temp_el->l_hatom);
								t_llllelem *next_temp_el = temp_el->l_next;
								if ((t_note *)temp_el->l_thing.w_obj == temp) {
									double this_cents = hatom_getdouble(&note_ll->l_head->l_hatom);
									double this_duration = hatom_getdouble(&note_ll->l_head->l_next->l_hatom);
									double this_velocity = note_ll->l_size >= 3 ? hatom_getdouble(&note_ll->l_head->l_next->l_next->l_hatom) : -1;

									// handling temporal slots' and breakpoints' information
									if (temp->duration <= 0) { // pathological case
										set_slots_values_to_note_from_llll((t_notation_obj *)x, temp, find_sublist_with_router((t_notation_obj *)x, hatom_getllll(&temp_el->l_hatom), _llllobj_sym_slots));
										set_breakpoints_values_to_note_from_llll((t_notation_obj *)x, temp, find_sublist_with_router((t_notation_obj *)x, hatom_getllll(&temp_el->l_hatom), _llllobj_sym_breakpoints));
									} else if (this_duration > 0){
										t_llll *temp_llll = hatom_getllll(&temp_el->l_hatom);
										t_llll *breakpoints = find_sublist_with_router((t_notation_obj *)x, temp_llll, _llllobj_sym_breakpoints);
										t_note *dummy_breakpoints_note = NULL;
										if (breakpoints) {
											dummy_breakpoints_note = build_note((t_notation_obj *) x, this_cents, this_duration, this_velocity);
											set_breakpoints_values_to_note_from_llll((t_notation_obj *)x, dummy_breakpoints_note, breakpoints);
										}
										double end_glued_note_portion_rel_x = CLAMP((parent->onset - onset)/this_duration, 0., 1.);
										double duration_ratio1 = (parent->onset - onset)/temp->duration;
										glue_portion_of_temporal_slots((t_notation_obj *)x, temp, temp_llll, 0., end_glued_note_portion_rel_x, duration_ratio1, -1, smooth_ms);
										glue_portion_of_breakpoints((t_notation_obj *)x, temp, temp_llll, dummy_breakpoints_note, 0., end_glued_note_portion_rel_x, duration_ratio1, -1, smooth_ms);
										if (this_duration + onset > parent->onset + temp->duration) {
											double start_glued_note_portion_rel_x = CLAMP((parent->onset + temp->duration - onset)/this_duration, 0., 1.);
											double duration_ratio2 = (onset + this_duration - (parent->onset + temp->duration))/(parent->onset + temp->duration - onset);
											glue_portion_of_temporal_slots((t_notation_obj *)x, temp, temp_llll, start_glued_note_portion_rel_x, 1., duration_ratio2, 1, smooth_ms);
											glue_portion_of_breakpoints((t_notation_obj *)x, temp, temp_llll, dummy_breakpoints_note, start_glued_note_portion_rel_x, 1., duration_ratio2, 1, smooth_ms);
										}
										if (dummy_breakpoints_note)
											free_note((t_notation_obj *)x, dummy_breakpoints_note);
									}
									
									// updating velocity
									if (this_velocity > 0)
										temp->velocity = CLAMP(((temp->velocity * temp->duration) + (this_velocity * this_duration))/(temp->duration + this_duration), CONST_MIN_VELOCITY, CONST_MAX_VELOCITY);

									// updating duration
									temp->duration = MAX(parent->onset + temp->duration, this_duration + onset) - onset;
									
									
									if (temp_el == next)
										next = next->l_next;
									llll_destroyelem(temp_el);
								}
								temp_el = next_temp_el;
							}
						}
						parent->onset = onset;
					}
					
				}
				
				note_elem = next;
			}
			
			for (note_elem = chord->l_head->l_next; note_elem; note_elem = note_elem->l_next)
				note_elem->l_thing.w_obj = NULL;
			
			
			if (get_num_llll_no_first_attribute_sym_in_llll(chord) > 0) { // if there are still some notes
				t_chord *ch = addchord_from_llll(x, chord, voice, true, true); 
				if (ch)
					create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)ch, k_UNDO_MODIFICATION_DELETE);
			}
		}
	}
}


void set_clefs_from_llll(t_roll *x, t_llll* clefs){
	if (clefs) {
		t_atom *av = NULL;
		long ac = llll_deparse(clefs, &av, 0, 1);
		roll_setattr_clefs(x, NULL, ac, av);
		if (av) bach_freeptr(av);
	}
}

void set_keys_from_llll(t_roll *x, t_llll* keys){
	if (keys) {
		t_atom *av = NULL;
		long ac = llll_deparse(keys, &av, 0, 1);
		roll_setattr_keys(x, NULL, ac, av);
		if (av) bach_freeptr(av);
	}
}


void set_groups_from_llll(t_roll *x, t_llll *groups_as_llll){
//	clear_all_groups((t_notation_obj *)x);
	if (groups_as_llll){
		t_llllelem *elem;
		for (elem = groups_as_llll->l_head; elem; elem = elem->l_next){
			if (hatom_gettype(&elem->l_hatom) == H_LLLL){
				t_llll *thisgroup = hatom_getllll(&elem->l_hatom);
				t_llllelem *el2;
				if (thisgroup->l_size > 1){
					t_group *newgroup = build_group();
					append_group((t_notation_obj *) x, newgroup);
					for (el2 = thisgroup->l_head; el2; el2 = el2->l_next){
						if (hatom_gettype(&el2->l_hatom) == H_LLLL){
							t_llll *thisllll = hatom_getllll(&el2->l_hatom);
							if (thisllll->l_size == 2 && hatom_gettype(&thisllll->l_head->l_hatom) == H_LONG && hatom_gettype(&thisllll->l_head->l_next->l_hatom) == H_LONG){
								long num_voice = hatom_getlong(&thisllll->l_head->l_hatom);
								long num_chord = hatom_getlong(&thisllll->l_head->l_next->l_hatom);
								if (num_voice >= 1 && num_voice <= x->r_ob.num_voices){
									t_rollvoice *voice = nth_rollvoice(x, num_voice - 1);
									if (voice && num_chord >= 1 && num_chord <= voice->num_chords) {
										t_chord *chord = nth_chord_of_rollvoice(voice, num_chord);
										if (chord && newgroup)
											append_element_in_group((t_notation_obj *) x, newgroup, (t_notation_item *)chord);
									}
								}
							}
						}
					}
					
					if (newgroup->num_elements <= 1) 
						delete_group((t_notation_obj *) x, newgroup);
				}
			}
		}
	}
}


void create_whole_roll_undo_tick(t_roll *x)
{
	if (x->r_ob.inhibited_undo)
		return;
	if (!(atom_gettype(&x->r_ob.max_undo_steps) == A_LONG && atom_getlong(&x->r_ob.max_undo_steps) == 0)) {
		t_llll *content = get_roll_values_as_llll(x, k_CONSIDER_FOR_UNDO, k_HEADER_ALL, true, true);
		// we clone the content outside the memory pool so that it does not fill it
		t_llll *content_cloned = llll_clone_extended(content, WHITENULL_llll, 1, NULL);
		t_undo_redo_information *operation = build_undo_redo_information(0, k_WHOLE_NOTATION_OBJECT, k_UNDO_MODIFICATION_CHANGE, 0, 0, k_HEADER_NONE, content_cloned);
		llll_free(content);
		create_undo_redo_tick((t_notation_obj *) x, k_UNDO, 0, operation, true);
	}
}

void set_roll_from_llll_from_read(t_roll *x, t_llll* inputlist)
{  
	if (inputlist) {
		create_whole_roll_undo_tick(x); 
		set_roll_from_llll(x, inputlist, true);
	}
}

void set_roll_from_llll(t_roll *x, t_llll* inputlist, char also_lock_general_mutex)
{
// set the whole roll, starting from a llll (it clones the llll)

	t_llll *wholeroll = llll_get();
	t_llll *groups = NULL;
	char markers_are_given = false;
	
	if (!inputlist) { 
		llll_free(wholeroll); 
		return; 
	}
	
	if (also_lock_general_mutex)
		lock_general_mutex((t_notation_obj *)x);	
	
	llll_clone_upon(inputlist, wholeroll);
	
	// if we have a "roll" message at the beginning, we get rid of it (ok, it's a "signature" that everything is ok)
	if (wholeroll->l_size > 0 && hatom_gettype(&wholeroll->l_head->l_hatom) == H_SYM) { 
		t_llllelem *firstelem = wholeroll->l_head; // this is a symbol
		t_symbol *firstsym = hatom_getsym(&firstelem->l_hatom);
		if (firstsym == _llllobj_sym_roll) 
			llll_destroyelem(firstelem);
		else if (firstsym == _llllobj_sym_score) {
			object_error((t_object *) x, "Can't load a bach.score content into a bach.roll. Use bach.score2roll instead.");
			llll_free(wholeroll);
			
			if (also_lock_general_mutex)
				unlock_general_mutex((t_notation_obj *)x);	
			
			return;
		} else if (firstsym == _llllobj_sym_slot){
			object_error((t_object *) x, "Can't load a bach.slot content into a bach.roll.");
			llll_free(wholeroll);
			
			if (also_lock_general_mutex)
				unlock_general_mutex((t_notation_obj *)x);	
			
			return;
		}
	}
	
	if (wholeroll->l_size > 0) { 
		t_llllelem *voiceelem;
		t_rollvoice *voice;
		
		// checking header
		while (true) { // cycle on the header elements
			t_llllelem *firstelem = wholeroll->l_head; // this has to be a linked list

			if (!firstelem) break; // there's no body content

			if (hatom_gettype(&firstelem->l_hatom) == H_LLLL) {
				t_llll *firstllll = hatom_getllll(&firstelem->l_hatom);
				if (firstllll->l_size > 0) {
					t_llllelem *pivot = firstllll->l_head;
					if (hatom_gettype(&pivot->l_hatom) == H_SYM) {
						// we're still in the header;
						t_symbol *pivotsym = hatom_getsym(&pivot->l_hatom);
						if (pivotsym == _llllobj_sym_slotinfo) {
							llll_destroyelem(pivot); // we kill the pivot, in order to give the correct llll to the set_slotinfo function
							if (firstllll && firstllll->l_head) {
								t_llll *slots_to_erase = set_slotinfo_from_llll((t_notation_obj *) x, firstllll);
								notationobj_erase_slots_from_llll((t_notation_obj *)x, slots_to_erase);
								llll_free(slots_to_erase);
							}
						} else if (pivotsym == _llllobj_sym_commands) {
							llll_destroyelem(pivot); // we kill the pivot, in order to give the correct llll to the function
							if (firstllll && firstllll->l_head)
								set_commands_from_llll((t_notation_obj *) x, firstllll);
						} else if (pivotsym == _llllobj_sym_midichannels) {
							llll_destroyelem(pivot);
							if (firstllll && firstllll->l_head)
								set_midichannels_from_llll((t_notation_obj *)x, firstllll);
						} else if (pivotsym == _llllobj_sym_stafflines) {
							llll_destroyelem(pivot);
							if (firstllll && firstllll->l_head)
								set_stafflines_from_llll((t_notation_obj *)x, firstllll, true);
						} else if (pivotsym == _llllobj_sym_clefs) {
							llll_destroyelem(pivot); 
							if (firstllll && firstllll->l_head)
								set_clefs_from_llll(x, firstllll);
						} else if (pivotsym == _llllobj_sym_keys) {
							llll_destroyelem(pivot); 
							if (firstllll && firstllll->l_head)
								set_keys_from_llll(x, firstllll);
						} else if (pivotsym == _llllobj_sym_voicenames) {
							llll_destroyelem(pivot); 
							set_voicenames_from_llll((t_notation_obj *)x, firstllll, true);
						} else if (pivotsym == _llllobj_sym_markers) {
							markers_are_given = true;
							llll_destroyelem(pivot); 
							set_markers_from_llll((t_notation_obj *)x, firstllll, false, false);
						} else if (pivotsym == _llllobj_sym_groups) {
							llll_destroyelem(pivot); 
							groups = llll_clone(firstllll);
                        } else if (pivotsym == _llllobj_sym_articulationinfo) {
                            llll_destroyelem(pivot);
                            if (firstllll && firstllll->l_head)
                                set_articulationinfo_from_llll((t_notation_obj *)x, &x->r_ob.articulations_typo_preferences, firstllll, false);
                        } else if (pivotsym == _llllobj_sym_noteheadinfo) {
                            llll_destroyelem(pivot);
                            if (firstllll && firstllll->l_head)
                                set_noteheadinfo_from_llll((t_notation_obj *)x, firstllll, false);
                        } else if (pivotsym == _llllobj_sym_numparts) {
                            llll_destroyelem(pivot);
                            if (firstllll && firstllll->l_head)
                                notation_obj_set_numparts_from_llll((t_notation_obj *)x, firstllll);
                        } else if (pivotsym == _llllobj_sym_loop) {
                            llll_destroyelem(pivot);
                            if (firstllll && firstllll->l_head)
                                set_loop_region_from_llll(x, firstllll, false);
						}
					} else 
						break; // break and go to the body of the object!
				} else 
					break; // break and go to the body of the object!
			}
			llll_destroyelem(firstelem); // we kill the first element and continue
		}
			
		// now we're ready to iterate on rollvoices
		voiceelem = wholeroll->l_head; 
		if (voiceelem && !x->must_append_chords) {
			// if there is some roll info, we clear the roll, as a very first thing.
			clear_roll_body(x, -1);
			if (x->r_ob.firstmarker && !markers_are_given) 
				clear_all_markers((t_notation_obj *) x);
			close_slot_window((t_notation_obj *)x);
			clear_selection((t_notation_obj *) x);
		}
		
		// autosizing the number of voices – NEW CODE: bach 0.7.4
		if (x->r_ob.autosize) {
			long num_voices = CLAMP(get_num_llll_in_llll_first_level(wholeroll), 0, CONST_MAX_VOICES);

			// we change the number of voices if there was no "addchords" message and the number of voice inserted is different from the existing one,
			// or if there was "addchords" message, and the number of voices inserted is greater than the existing ones.
			if (num_voices > 0 && !x->pasting_chords && // if numvoices == 0 it means that there's only header in the incoming llll, if pasting_chords is true it means that we're copy pasting: no voice change in this case!
				((!x->must_append_chords && x->r_ob.num_voices != num_voices) ||
				 (x->must_append_chords && x->r_ob.num_voices < num_voices)))
				set_numvoices((t_notation_obj *)x, num_voices);
		}
		
		voice = x->firstvoice;
		while (voiceelem && voice){
			// is it a suitable llll for a voice?
			if (hatom_gettype(&voiceelem->l_hatom) == H_LLLL) {
				t_llll *voiceelemllll = hatom_getllll(&voiceelem->l_hatom);

				if (voiceelemllll->l_size > 0) {
					
					t_llllelem *elem = voiceelemllll->l_head;
					if (voice) {
						t_chord *chord;

						long forced_voice_ID = notation_item_get_ID_from_llll(voiceelemllll);
						if (forced_voice_ID) {
							shashtable_chuck_thing(x->r_ob.IDtable, voice->v_ob.r_it.ID);
							shashtable_insert_with_key(x->r_ob.IDtable, voice, forced_voice_ID, 1);
							voice->v_ob.r_it.ID = forced_voice_ID;
						}
						
						// Old non-symbolic mute/lock flag setting mechanism
                        if (voiceelemllll->l_tail && hatom_gettype(&voiceelemllll->l_tail->l_hatom) != H_LLLL)
                            notation_item_set_flags_from_llllelem((t_notation_obj *) x, voiceelemllll->l_tail, (t_notation_item *)voice, true);
						
#ifdef BACH_OUTPUT_SYMBOLIC_FLAGS
                        notation_item_find_and_set_flags((t_notation_obj *) x, (t_notation_item *)voice, voiceelemllll);
#endif
                        
						// now we're ready to iterate on chords
						chord = x->must_append_chords ? NULL : voice->firstchord;
						while (elem){
							// is it a suitable llll for a chord?
							if (hatom_gettype(&elem->l_hatom) == H_LLLL) {
								t_llll *elemllll = hatom_getllll(&elem->l_hatom);
								unsigned long forced_chord_ID = notation_item_get_ID_from_llll(elemllll);

								if (elemllll->l_size >= 2) { // there has to be at least (onset ( NOTE ) )
									if (chord) { // there's already a chord
										set_rollchord_values_from_llll((t_notation_obj *) x, chord, elemllll, 0., true, false, false); // MUST NOT CHECK NOTES ORDER!!! Only at the end, otherwise it's a mess
										chord->need_recompute_parameters = true; // we have to recalculate chord parameters 
										chord = chord->next;
									} else { // there was no chord, we gotta create one!
										double onset = voice->lastchord ? (voice->lastchord->prev ? 2 * voice->lastchord->onset - voice->lastchord->prev->onset: voice->lastchord->onset + CONST_DEFAULT_NEW_NOTE_ONSET) : 0.;
										long num_notes = get_num_llll_no_first_attribute_sym_in_llll(elemllll); //elemllll->l_size - 1;
										t_llllelem *elem = elemllll->l_head;
										double *argv = (double *) bach_newptr(2 * num_notes * sizeof(double));
										unsigned long *forced_note_IDs = (unsigned long *) bach_newptr(MAX(1, num_notes) * sizeof(unsigned long));
										t_chord *newchord;
										long i = 0;
										
										if (hatom_gettype(&elem->l_hatom) != H_LLLL)
											elem = get_next_llll_no_first_attribute_sym_elem(elem);
										
										for (i = 0; elem && i < num_notes; i++, elem = get_next_llll_no_first_attribute_sym_elem(elem)) {
											argv[2 * i] = voice->lastchord && voice->lastchord->lastnote ? voice->lastchord->lastnote->duration : CONST_DEFAULT_NEW_NOTE_DURATION; 
											argv[2 * i + 1] = CONST_DEFAULT_NEW_NOTE_CENTS;
											forced_note_IDs[i] = !elem ? 0 : notation_item_get_ID_from_llll(hatom_getllll(&elem->l_hatom));
										}
										
										newchord = addchord_from_values(x, voice->v_ob.number, num_notes, onset, -1, 2 * num_notes, argv, NULL, NULL, 0, NULL, false, forced_chord_ID, forced_note_IDs, false);
										if (newchord) {
											set_rollchord_values_from_llll((t_notation_obj *) x, newchord, elemllll, x->must_apply_delta_onset, true, false, false); // MUST NOT CHECK NOTES ORDER!!! Only at the end, otherwise it's a mess
											newchord->need_recompute_parameters = true; // we have to recalculate chord parameters 
											if (x->must_append_chords){
												create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)newchord, k_UNDO_MODIFICATION_DELETE);
												if (x->must_preselect_appended_chords)
													notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)newchord);
											}
										}
										bach_freeptr(argv);
										bach_freeptr(forced_note_IDs);
									}
								}
							}
							// else: we completely ignore it!
							elem = elem->l_next;
						}
					}
				}
				voice = voice->next;
			}
			voiceelem = voiceelem->l_next;
		}
	}

	// setting groups
	if (groups){
		clear_all_groups((t_notation_obj *) x);
		set_groups_from_llll(x, groups);
		llll_free(groups);
	}
	
	x->must_append_chords = false;
	x->must_preselect_appended_chords = false;
	x->must_apply_delta_onset = 0.;
	check_all_chords_order(x); // this has to be done ONLY at the end! otherwise, it's a mess! :-)
	x->r_ob.are_there_solos = are_there_solos((t_notation_obj *) x);
	recompute_total_length((t_notation_obj *)x);
	
	process_chord_parameters_calculation_NOW(x);
	
	if (also_lock_general_mutex)
		unlock_general_mutex((t_notation_obj *)x);	
	
	check_correct_scheduling((t_notation_obj *)x, also_lock_general_mutex);
	
	llll_free(wholeroll);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	verbose_print(x);
}


void process_chord_parameters_calculation_NOW(t_roll *x){
	t_rollvoice *voice;
	t_jfont *jf_lyrics_nozoom = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, x->r_ob.lyrics_font_size);
    t_jfont *jf_dynamics_nozoom = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, x->r_ob.dynamics_font_size);

	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
		t_chord *curr_ch;
		for (curr_ch = voice->firstchord; curr_ch; curr_ch = curr_ch->next){
			if (curr_ch->need_recompute_parameters) {
				assign_chord_lyrics((t_notation_obj *) x, curr_ch, jf_lyrics_nozoom);
                assign_chord_dynamics((t_notation_obj *) x, curr_ch, jf_dynamics_nozoom);
				calculate_chord_parameters((t_notation_obj *) x, curr_ch, get_voice_clef((t_notation_obj *)x, (t_voice *)voice), true);
				curr_ch->need_recompute_parameters = false;
			}
		}
	}
	jfont_destroy_debug(jf_lyrics_nozoom);
    jfont_destroy_debug(jf_dynamics_nozoom);
}


void snap_pitch_to_grid_voice(t_roll *x, t_rollvoice *voice) {
	t_chord *curr_ch = voice->firstchord;
	while(curr_ch){ // cycle on the chords
		t_note *curr_nt = curr_ch->firstnote; 
		while(curr_nt){ // cycle on the chords
			snap_pitch_to_grid_for_note((t_notation_obj *) x, curr_nt);
			curr_nt = curr_nt->next;
		}
		curr_ch->need_recompute_parameters = true; // we have to recalculate chord parameters 
		curr_ch = curr_ch->next;
	}
}

void roll_snap_pitch_to_grid(t_roll *x, t_symbol *s, long argc, t_atom *argv){
// retranscribe the midicents, with respect to the shown approximation on the score, and NOT to the fine tuning. 
	if (argc >= 0) { // retranscribe only a voice
		long numvoice = atom_getlong(argv) - 1; // 0-based inside, 1-based for the user
		t_rollvoice *voice = nth_rollvoice(x, numvoice);
		lock_general_mutex((t_notation_obj *)x);
		snap_pitch_to_grid_voice(x, voice);
		unlock_general_mutex((t_notation_obj *)x);
	} else {
		t_rollvoice *voice = x->firstvoice;
		lock_general_mutex((t_notation_obj *)x);
		while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
			snap_pitch_to_grid_voice(x, voice);
			voice = voice->next;
		}
		unlock_general_mutex((t_notation_obj *)x);
	}
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
}

void roll_getmaxID(t_roll *x){
	object_post((t_object *) x, "Last used ID: %ld", x->r_ob.IDtable->s_lastused);
}

// voicenum >= 0: 0-based specific voice
// voicenum = -1: all voices up to current voice number
// voicenum = -2: all voices up to CONST_MAX_VOICES
void clear_roll_body(t_roll *x, long voicenum){
// clears all the chords

	clear_preselection((t_notation_obj *) x);
	clear_selection((t_notation_obj *) x);
	close_slot_window((t_notation_obj *)x); // if we were in slot view...

	clear_all_groups((t_notation_obj *) x);
	
	if (voicenum >= 0) { // there's a voice argument
		t_rollvoice *voice = nth_rollvoice(x, voicenum);
		t_chord *chord = voice->firstchord;
		t_chord *temp = chord; 
		while (chord) {
			temp = chord->next;
			delete_chord_from_voice((t_notation_obj *)x, chord, NULL, false);
			chord = temp;
		}
	} else {
		t_rollvoice *voice = x->firstvoice;
		long max_voice_to_clear = voicenum == -2 ? CONST_MAX_VOICES : x->r_ob.num_voices;
		while (voice && (voice->v_ob.number < max_voice_to_clear)) {
			t_chord *chord = voice->firstchord;
			t_chord *temp = chord; 
			while (chord) {
				temp = chord->next;
				delete_chord_from_voice((t_notation_obj *)x, chord, NULL, false);
				chord = temp;
			}
			voice = voice->next;
			
		}
	}
	
	check_correct_scheduling((t_notation_obj *)x, false);

	recompute_total_length((t_notation_obj *)x);

	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
}

void roll_clear_all(t_roll *x)
{
    clear_all_markers((t_notation_obj *)x); // must be BEFORE clear_roll_body, because inside clear_roll_body we also recompute total length
	clear_roll_body(x, -2);
}

void verbose_print(t_roll *x){
	if (verbose){
		t_notation_item *temp;
		long count1, count2;
		t_rollvoice *voice = x->firstvoice;
		t_chord *curr_ch;
		post("There are %ld ROLLVOICES. First rollvoice: %lx. Last rollvoice: %lx", x->r_ob.num_voices, x->firstvoice, x->lastvoice);
		while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
			post("**** VOICE %ld ****", voice->v_ob.number);
			curr_ch = voice->firstchord;
			count1 = 0;
			post("There are %ld CHORDS. First chord: %lx. Last chord: %lx", voice->num_chords, voice->firstchord, voice->lastchord);
			while (curr_ch) {
				t_note *curr_nt;
				count1++;
				post(" - Chord #%ld. Address: %lx. Onset: %.1fms. Prev: %lx, Next: %lx. It has %ld notes. First note: %lx. Last note: %lx", count1, curr_ch, curr_ch->onset, curr_ch->prev, curr_ch->next, curr_ch->num_notes, curr_ch->firstnote, curr_ch->lastnote);  
				curr_nt = curr_ch->firstnote; 
				count2 = 0;
				while (curr_nt) {
					count2++;
					post("     . Note #%ld. Address: %lx. Parent: %lx. Prev: %lx, Next: %lx. Dur: %.1f, mc: %.1f, vel: %.d, acc: %ld/%ld, def_acc: %d. notecenter_stem_delta_ux: %f", count2, curr_nt, curr_nt->parent, curr_nt->prev, curr_nt->next, curr_nt->duration, curr_nt->midicents, curr_nt->velocity, curr_nt->pitch_original.alter().r_num, curr_nt->pitch_original.alter().r_den, note_is_enharmonicity_userdefined(curr_nt), curr_nt->notecenter_stem_delta_ux);
					curr_nt = curr_nt->next;
				}
				curr_ch = curr_ch->next;
			}
			voice = voice->next;
		}
		
		post("%ld elements SELECTED. First element: %lx. Last element: %lx", x->r_ob.num_selecteditems, x->r_ob.firstselecteditem, x->r_ob.lastselecteditem);
		temp = x->r_ob.firstselecteditem; 
		count1 = 0;
		while (temp){
			count1++;
			post("- Element #%ld. Element address: %lx, type: %ld.", count1, temp, temp->type);
			temp = temp->next_selected;
		}
	}
}

t_note* merge_chords(t_roll *x, t_chord *chord1, t_chord *chord2, char add_cloned_items_to_preselection, char transfer_mousedown, char assign_IDs){
// merge the notes of the second chord in the first one. VERY IMPORTANT: the algorithm _frees the second chord_ after merging.
// we just append the list of notes, and then check the order, in order to re-arrange correctly the notes (sorted by midicents, from min to max)
// returns the pointer to the newly j_mousedown note, if any

	t_note *new_mousedown_note = NULL;
	if (chord2 && chord1 && chord2->firstnote) {
		long mousedown_id_in_chord1 = -1, i;
		t_note *temp_nt = chord2->firstnote, *temp_prev = chord1->lastnote;
		
		if (transfer_mousedown) {
			t_note *tmp;
			for (tmp = chord1->firstnote, i = 0; tmp; tmp = tmp->next, i++){
				if (x->r_ob.j_mousedown_ptr == tmp || (x->r_ob.j_mousedown_ptr == chord1 && chord1->num_notes == 1)){
					mousedown_id_in_chord1 = i;
					break;
				}
			}
		}

		i = 0;
		while (temp_nt) {
			t_note *temp_nt2 = clone_note((t_notation_obj *) x, temp_nt, k_CLONE_FOR_ORIGINAL);
			
			// j_mousedown to be transfered?
			if (transfer_mousedown && !new_mousedown_note && mousedown_id_in_chord1 == i)
				new_mousedown_note = temp_nt2;
			
			if (assign_IDs)
				temp_nt2->r_it.ID = shashtable_insert(x->r_ob.IDtable, temp_nt2);

			temp_nt2->parent = chord1; // change parent!!!
			chord1->num_notes++;
			temp_nt2->prev = temp_prev;
			if (temp_prev) // NOT the first note
				temp_prev->next = temp_nt2;
			else //first note
				chord1->firstnote = temp_nt2;
			temp_prev = temp_nt2;
			temp_nt = temp_nt->next;
			if (add_cloned_items_to_preselection) // we could drop this, but it's useful when we drag'n'drop, so that just the NEW notes remain selected
				notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)temp_nt2); 
			i++;
		}
		if (temp_prev) {
			temp_prev->next = NULL;
			chord1->lastnote = temp_prev;
		}
		
		chord_check_dependencies_before_deleting_it((t_notation_obj *)x, chord2, false, NULL);
		free_chord((t_notation_obj *)x, chord2);
	} 
	
	check_notes_order(chord1);
	return new_mousedown_note;
}


void append_chord(t_roll *x, t_rollvoice *voice, t_chord *chord, unsigned long force_ID){
	if (voice && chord){
		voice->num_chords++; // increase the # of chords
		if (voice->lastchord) {
			chord->next = NULL; 
			chord->prev = voice->lastchord;
			voice->lastchord->next = chord;
			voice->lastchord = chord;
		} else {
			// first chord in the score
			chord->prev = NULL;
			chord->next = NULL;
			voice->firstchord = chord;
			voice->lastchord = chord;
		}
		if (force_ID > 0) {
			shashtable_insert_with_key(x->r_ob.IDtable, chord, force_ID, 1);
			chord->r_it.ID = force_ID;
		} else
			chord->r_it.ID = shashtable_insert(x->r_ob.IDtable, chord);
	}
}

void insert_chord(t_roll *x, t_rollvoice *voice, t_chord *chord, unsigned long force_ID){
// insert an (already built) chord in the linked list.
// find the correct position of the chord in the linked list, with respect to the onset
	if (voice && chord) {
		voice->num_chords++; // increase the # of chords

		chord->voiceparent = voice; // set parent
		
		if (voice->firstchord) { // there is already at least a chord: insert the chord in the score
			if (voice->firstchord->onset > chord->onset) { // gotta put it at the first position
				chord->next = voice->firstchord;
				chord->prev = NULL;
				voice->firstchord->prev = chord;
				voice->firstchord = chord;
			} else if (voice->lastchord->onset < chord->onset) { // gotta put it at the end!
				voice->num_chords--; // the number of chords will be re-increased by append_chord
				append_chord(x, voice, chord, force_ID);
			} else { // it's in the middle
				// trying to find the correct positioning
				
				if (voice->lastchord->onset - chord->onset < chord->onset - voice->firstchord->onset) {
					// start from the end
					t_chord *temp_ch = voice->lastchord;
					while (temp_ch){ // cycle on the chords
						if (temp_ch->onset < chord->onset){
							// insert the chord in the list
							chord->prev = temp_ch;
							chord->next = temp_ch->next;
							temp_ch->next = chord;
							chord->next->prev = chord;
							break;
						}
						temp_ch = temp_ch->prev;
					}
					if (!temp_ch) { // it means that the cycle hasn't been broken, so we gotta put the chord at the BEGINNING of the chain
						chord->next = voice->firstchord;
						chord->prev = NULL;
						voice->firstchord->prev = chord;
						voice->firstchord = chord;
					}
				} else {
					// start from the beginning
					t_chord *temp_ch = voice->firstchord;
					while(temp_ch){ // cycle on the chords
						if (temp_ch->onset > chord->onset){
							// insert the chord in the list
							chord->next = temp_ch;
							chord->prev = temp_ch->prev;
							temp_ch->prev = chord;
							chord->prev->next = chord;
							break;
						}
						temp_ch = temp_ch->next;
					}
					
					if (!temp_ch) { // it means that the cycle hasn't been broken, so we gotta put the chord at the END of the chain
						voice->num_chords--; // the number of chords will be re-increased by append_chord
						append_chord(x, voice, chord, force_ID);
					}
				}
				
			}
		} else { // first chord in the score!
			// creates a new chord
			chord->prev = NULL;
			chord->next = NULL;
			voice->firstchord = chord;
			voice->lastchord = chord;
		}
		if (force_ID > 0) {
			shashtable_insert_with_key(x->r_ob.IDtable, chord, force_ID, 1);
			chord->r_it.ID = force_ID;
		} else
			chord->r_it.ID = shashtable_insert(x->r_ob.IDtable, chord);
	}
}

// force_append = 1 forces the chord to be put at the end of the list, otherwise it is put in the correct position w.r. to ms
t_chord* addchord_from_notes(t_roll *x, long voicenumber, double onset, long unused, long num_notes, 
								t_note *firstnote, t_note *lastnote, char force_append, unsigned long force_ID) {
// add a chord, starting from a linked list of notes

	t_rollvoice *voice;
	t_chord *this_ch;
	t_note *curr_nt;
	
	voice = nth_rollvoice(x, voicenumber);
	
	// create chord, and insert it in the linked list
	this_ch = (t_chord *)bach_newptrclear(sizeof(t_chord));
	notation_item_init(&this_ch->r_it, k_CHORD);
	this_ch->lyrics = build_lyrics(this_ch);
    this_ch->dynamics = build_dynamics(this_ch);

	this_ch->just_added_from_separate_parameters = false;
	this_ch->onset = onset;
	this_ch->need_recompute_parameters = false;
	this_ch->voiceparent = voice;
	this_ch->parent = NULL;
	this_ch->is_score_chord = false;
	this_ch->system_index = 0;
	this_ch->num_notes = num_notes;
	this_ch->rhythmic_tree_elem = NULL;
	this_ch->overall_tuplet_ratio = long2rat(1);
	this_ch->firstnote = firstnote;
	this_ch->lastnote = lastnote;
	this_ch->played = false;
	this_ch->muted = false;
	this_ch->locked = false;
	this_ch->solo = false;
	this_ch->is_grace_chord = false;
	this_ch->dont_split_for_ts_boxes = false;
	this_ch->imposed_direction = 0;

	// unused by roll, but set by default
	this_ch->r_sym_duration = RAT_1OVER4;
	this_ch->r_sym_onset = long2rat(0);
	
	this_ch->num_articulations = 0; 
	this_ch->articulation = NULL;

	if (force_append)
		append_chord(x, voice, this_ch, force_ID);
	else
		insert_chord(x, voice, this_ch, force_ID);
	
	recompute_total_length((t_notation_obj *)x); 
	verbose_print(x);
	
	// compute the approximation
	curr_nt = this_ch->firstnote;  
	while(curr_nt) {
		curr_nt->parent = this_ch;
		curr_nt = curr_nt->next;
	}

	this_ch->need_recompute_parameters = true; // gotta calculate chord parameters
	
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	
	return this_ch;
}

t_chord* addchord_from_values(t_roll *x, long voicenumber, long num_notes, double onset, long unused, long argc, double *argv, long* num_bpt, double *bpt, 
								long slot_list_length, t_atom *slots, char force_append, unsigned long force_ID, unsigned long *force_note_IDs, char also_recompute_total_length) {
								
// add a chord. Possible arguments are: [...] = looped numnotes-times.
// - voicenumber numnotes onset ID [duration midicents]
// - voicenumber numnotes onset ID [duration midicents velocity]
// - voicenumber numnotes onset ID [duration midicents velocity accidentals]
// - voicenumber numnotes onset ID [duration midicents velocity accidentals flag]
// - voicenumber numnotes onset ID [duration midicents velocity accidentals flag ID]
// plus: num_bpt (sized num_notes) contains the number of breakpoints (one entry for each note) - leave NULL for no breakpoint info
//		 all the breakpoints are then listed (rel_x, y, slope)
// NB:: leave ID = -1 for autoassign!!

	t_rollvoice *voice = nth_rollvoice(x, voicenumber);
	
	// argv[0] argv[1] argv[2] argv[3] argv[4] argv[5] argv[6] argv[7] argv[8] argv[9] argv[10] argv[11]  num_bpt[0] num_bpt[1] bpt[0] bpt[1] bpt[2] bpt[3] bpt[4] bpt[5]
	if (num_notes > 0) {
		if ((argc !=2*num_notes) && (argc!=3*num_notes) && (argc!=4*num_notes) && (argc!=5*num_notes) && (argc!=6*num_notes)) {
			object_warn((t_object *)x, "Can't build a chord with these parameters.");
			return NULL;
		} else {
			
			// assign ID if needed
			t_chord *this_ch;
			long i;
			t_note *head_nt = NULL;
			long k; long pos = 0;
			t_note *temp;
			int j;
			long s = 0; // lecture head... 
			
			// create chord, and insert it in the linked list
			this_ch = (t_chord *)bach_newptrclear(sizeof(t_chord));
			
			notation_item_init(&this_ch->r_it, k_CHORD);
			this_ch->lyrics = build_lyrics(this_ch);
            this_ch->dynamics = build_dynamics(this_ch);

			this_ch->imposed_direction = 0;
			this_ch->just_added_from_separate_parameters = false;
			this_ch->voiceparent = voice;
			this_ch->parent = NULL;
			this_ch->is_score_chord = false;
			this_ch->system_index = 0;
			this_ch->need_recompute_parameters = false;
			this_ch->onset = onset;
			this_ch->played = false;
			this_ch->muted = false;
			this_ch->locked = false;
			this_ch->solo = false;
			this_ch->num_notes = num_notes;
			this_ch->rhythmic_tree_elem = NULL;
			this_ch->overall_tuplet_ratio = long2rat(1);
			this_ch->is_grace_chord = false;
			this_ch->dont_split_for_ts_boxes = false;

			// unused by roll, but set by default
			this_ch->r_sym_duration = RAT_1OVER4;
			this_ch->r_sym_onset = long2rat(0);

			this_ch->num_articulations = 0;
			this_ch->articulation = NULL;

			if (force_append)
				append_chord(x, voice, this_ch, force_ID);
			else
				insert_chord(x, voice, this_ch, force_ID);

			// create the chain of notes
			for (i = num_notes - 1; i >= 0; i--) { //reversed cycle on the notes (each note is put at the beginning of the list)
				t_note *this_note = NULL;
				// building the note
				if (argc == 2 * num_notes) // case (duration midicents)
					this_note = build_note_from_ac_av((t_notation_obj *)x, 2, argv+2*i);
				else if (argc == 3 * num_notes) // case (duration midicents velocity)
					this_note = build_note_from_ac_av((t_notation_obj *)x, 3, argv+3*i);
				else if (argc == 4 * num_notes) // cas(t_notation_obj *)x, e (duration midicents velocity accidental)
					this_note = build_note_from_ac_av((t_notation_obj *)x, 4, argv+4*i);
				else if (argc == 5 * num_notes) // case (duration midicents velocity accidental def_acc)
					this_note = build_note_from_ac_av((t_notation_obj *)x, 5, argv+5*i);
				else if (argc == 6 * num_notes) // case (duration midicents velocity accidental def_acc ID)
					this_note = build_note_from_ac_av((t_notation_obj *)x, 6, argv+6*i);
				
				this_note->parent = this_ch;
				this_note->next = head_nt;
				
#ifdef BACH_NOTES_HAVE_ID
				if (force_note_IDs && force_note_IDs[i] > 0) { 
					shashtable_insert_with_key(x->r_ob.IDtable, this_note, force_note_IDs[i], 1);
					this_note->r_it.ID = force_note_IDs[i];
				} else
					this_note->r_it.ID = shashtable_insert(x->r_ob.IDtable, this_note);
#endif
				
				if (i==num_notes-1) 
					this_ch->lastnote = this_note;
				if (i<num_notes-1) 
					head_nt->prev = this_note;
				head_nt = this_note;
			}
			this_ch->firstnote = head_nt;
			
			// handle breakpoints
			temp = this_ch->firstnote; 
			i = 0;
			while (temp) { // num_bpt[0] bpt[0] bpt[1] bpt[2] bpt[3] bpt[4] bpt[5]
				if ((num_bpt) && (num_bpt[i]>0)) {
					temp->num_breakpoints = 2;
					pos += 3; // skip first breakpoint (trivial)
					for (k = 1; k < num_bpt[i] - 1; k++) { // first breakpoint is standard... we don't copy it! last breakpoint is dealt separately with.
						add_breakpoint((t_notation_obj *) x, temp, bpt[pos], bpt[pos+1], bpt[pos+2], 0, CONST_DEFAULT_NEW_NOTE_VELOCITY, true);
						pos += 3;
					}
					// change last bpt values
					temp->lastbreakpoint->delta_mc = bpt[pos+1];
					temp->lastbreakpoint->slope = bpt[pos+2];
					pos += 3;
				}
				temp = temp->next; i++;
			}
			
			// handle slots
			// ALL THIS PART IS DEPRECATED, BETTER USE set_slots_values_to_note_from_llll();
			temp = this_ch->firstnote;  
			while (temp) { 
				// slots - first: tabula rasa
				for (i = 0; i< CONST_MAX_SLOTS; i++) {
					temp->slot[i].firstitem = NULL;
#ifdef BACH_SLOTS_HAVE_LASTITEM
					temp->slot[i].lastitem = NULL;
#endif
#ifdef BACH_SLOTS_HAVE_OWNER
					temp->slot[i].owner = temp;
#endif
#ifdef BACH_SLOTS_HAVE_ACTIVEITEM
					temp->slot[i].activeitem = NULL;
#endif
					temp->slot[i].length = 0;
				}
				
				if (slot_list_length > 0) {
					// then: parsing of *slots
					while (true) {
						if ((atom_gettype(slots + s) == A_SYM) && (strcmp(atom_getsym(slots + s)->s_name, "next") == 0)) { // go to next note!
							s += 1;
							break;			
						} else if ((atom_gettype(slots + s) == A_SYM) && (strcmp(atom_getsym(slots + s)->s_name, "slot") == 0)) { // slot content
							j = atom_getlong(slots + s + 1) - 1; // slot number (we make it is 0-based)
							if (atom_gettype(slots + s + 2) == A_SYM) { // it MUST be a A_SYM
								if (strcmp(atom_getsym(slots + s + 2)->s_name, "function") == 0) { // slot function
									long num_pts = atom_getlong(slots + s + 3);
									temp->slot[j].length = 0; // we'll put it at the end (just to avoid that append_slotitem get crazy)
									s += 4;
									for (i = 0; i < num_pts; i++) { // add each point 
										double x_val, y_val, slope;
										t_pts *point;
										t_slotitem *thisitem = build_slotitem((t_notation_obj *)x, &(temp->slot[j]));
										x_val = atom_getfloat(slots + s);
										y_val = atom_getfloat(slots + s + 1);
										slope = atom_getfloat(slots + s + 2);
										s += 3;
										point = (t_pts *)bach_newptr(sizeof(t_pts));
										point->x = x_val; point->y = y_val; point->slope = slope;
										thisitem->item = point;
										append_slotitem(thisitem); // points are ordered! we just have to append them
									}
									temp->slot[j].length = num_pts; // number of points
								} else if (strcmp(atom_getsym(slots + s + 2)->s_name, "long") == 0) { // slot long
									t_slotitem *thisitem = build_slotitem((t_notation_obj *)x, &(temp->slot[j]));
									long *val = (long *)bach_newptr(sizeof(long));
									temp->slot[j].length = 0;
									*val = atom_getlong(slots + s + 3);
									s += 4;
									thisitem->item = val;
									append_slotitem(thisitem);
								} else if (strcmp(atom_getsym(slots + s + 2)->s_name, "float") == 0) { // slot float
									t_slotitem *thisitem = build_slotitem((t_notation_obj *)x, &(temp->slot[j]));
									double *val = (double *)bach_newptr(sizeof(double));
									*val = atom_getfloat(slots + s + 3);
									s += 4;
									thisitem->item = val;
									temp->slot[j].length = 0;
									append_slotitem(thisitem);
								} else if (strcmp(atom_getsym(slots + s + 2)->s_name, "longlist") == 0) { // slot longlist
									long num_numbers = atom_getlong(slots + s + 3); // number of numbers
									temp->slot[j].length = 0;
									s += 4;
									for (i = 0; i < num_numbers; i++) { // add each number 
										t_slotitem *thisitem = build_slotitem((t_notation_obj *)x, &(temp->slot[j]));
										long *val = (long *)bach_newptr(sizeof(long));
										*val = atom_getlong(slots + s);
										s += 1;
										thisitem->item = val;
										append_slotitem(thisitem);
									}
								} else if (strcmp(atom_getsym(slots + s + 2)->s_name, "floatlist") == 0) { // slot floatlist
									long num_numbers = atom_getlong(slots + s + 3); // number of numbers
									temp->slot[j].length = 0;
									s += 4;
									for (i = 0; i < num_numbers; i++) { // add each number 
										t_slotitem *thisitem = build_slotitem((t_notation_obj *)x, &(temp->slot[j]));
										double *val = (double *)bach_newptr(sizeof(double));
										*val = atom_getfloat(slots + s);
										s += 1;
										thisitem->item = val;
										append_slotitem(thisitem);
									}
								} else if (strcmp(atom_getsym(slots + s + 2)->s_name, "text") == 0) { // slot text
									long num_chars = atom_getlong(slots + s + 3); // number of files
									t_slotitem *thisitem = build_slotitem((t_notation_obj *)x, &(temp->slot[j]));
									char *text = (char *) bach_newptr(num_chars + 1); 
									snprintf_zero(text, num_chars, atom_getsym(slots + 4)->s_name);
									s += 5;
									bach_copyptr(thisitem->item, text, num_chars * sizeof(char));
									temp->slot[j].length = 0; // just to avoid that append_slotitem (which incrase automatically the # of items) get crazy
									append_slotitem(thisitem);
									temp->slot[j].length = num_chars;
									bach_freeptr(text);
								} else if (strcmp(atom_getsym(slots + s + 2)->s_name, "filelist") == 0) { // slot filelist
									long num_files = atom_getlong(slots + s + 3); // number of files
									long act_file_index = atom_getlong(slots + s + 4); // active file index (1-based)
									temp->slot[j].length = 0;
									s += 5;
									for (i = 0; i < num_files; i++) { // add each file
										t_file *file;
										long pathID, filename_length;
										char filename[MAX_PATH_CHARS]; 
										t_slotitem *thisitem = build_slotitem((t_notation_obj *)x, &(temp->slot[j]));
										pathID = atom_getlong(slots + s);
										filename_length = atom_getlong(slots + s + 1);
										strncpy_zero(filename, atom_getsym(slots + s + 2)->s_name, MAX_PATH_CHARS);
										s += 3;
										file = (t_file *)bach_newptr(sizeof(t_file));
										file->exists = true;
										strncpy_zero(file->filename, filename, MAX_PATH_CHARS);
										file->pathID = pathID;
										file->filename_length = filename_length;
										thisitem->item = file;
										if (i + 1 == act_file_index)
                                            slot_set_active_item(&temp->slot[j], thisitem);
										append_slotitem(thisitem); // files are ordered! we just have to append them
									}
								} else { // undefined or wrongly defined slot
									s += 3;
								}
							}
						} else {
							s += 1;
							if (s == slot_list_length) break;
						}
					}
				}
					
				temp = temp->next;
			}

			if (also_recompute_total_length)
				recompute_total_length((t_notation_obj *)x); 
//			verbose_print(x);

			this_ch->need_recompute_parameters = true; // we have to recalculate chord parameters
			return this_ch; 
		}
	}
	
/*	t_note *curr_nt2 = x->firstchord->firstnote; // verbose
	while (curr_nt2) {
		post("- duration: %ld, midicents: %ld, velocity: %ld", curr_nt2->duration, curr_nt2->midicents, curr_nt2->velocity);
		curr_nt2 = curr_nt2->next;
	}*/

	return NULL; // if everything is ok, it will not get here 
}
	
// DEPRECATED!!!!!
void roll_addchord_from_values(t_roll *x, t_symbol *s, long argc, t_atom *argv){
// add a chord. Possible arguments are:
// - numnotes onset (duration midicents)
// - numnotes onset (duration midicents velocity)
// - numnotes onset (duration midicents velocity accidentals)
// - numnotes onset (duration midicents velocity accidentals flag)

	if (argc>=2){
		long this_num_notes, i; 
		double this_onset;
		double *argv_long = (double *) bach_newptr((argc-2) * sizeof(double));
		this_num_notes = atom_getlong(argv);
		this_onset = atom_getfloat(argv + 1);
		for (i=0; i<argc-2; i++)
			argv_long[i] = atom_getfloat(argv+i+2);
		lock_general_mutex((t_notation_obj *)x);	
		addchord_from_values(x, 0, this_num_notes, this_onset, -1, argc-2, argv_long+2, NULL, NULL, 0, NULL, false, 0, NULL, true);
		unlock_general_mutex((t_notation_obj *)x);	
		bach_freeptr(argv_long);
	}
} 

char merge(t_roll *x, double threshold_ms, double threshold_cents, char gathering_policy_ms, char gathering_policy_cents, char only_selected, char markers_also)
{
	char changed = 0;
	t_rollvoice *voice;	t_chord *chord; t_note *note;

	for (voice = x->firstvoice; voice && (voice->v_ob.number < x->r_ob.num_voices); voice = voice->next) {
		for (chord = voice->firstchord; chord; chord = chord->next){

			if (only_selected && !notation_item_is_globally_selected((t_notation_obj *)x, (t_notation_item *)chord))
				continue;
			
			if (threshold_ms >= 0.) { // time merging
				long count = 1; 
				double gathering_average_onset = chord->onset; 
				double last_onset = chord->onset;
				t_chord *chord2 = chord->next;
				char merged = false;
				while (chord2 && (chord2->onset - chord->onset <= threshold_ms)) {
					if (!only_selected || notation_item_is_globally_selected((t_notation_obj *)x, (t_notation_item *)chord2)) {
						t_chord *chord_to_merge;
						count++;
						gathering_average_onset += chord2->onset;
						last_onset = chord2->onset;
						// we merge the chord to the first one
						chord_to_merge = clone_chord((t_notation_obj *) x, chord2, k_CLONE_FOR_ORIGINAL);
						merge_chords(x, chord, chord_to_merge, false, true, true);
						delete_chord_from_voice((t_notation_obj *)x, chord2, NULL, false);
						merged = true;
					}
					chord2 = chord2->next;
				}

				if (merged)
					check_correct_scheduling((t_notation_obj *)x, false);

				gathering_average_onset /= count;
				if (count > 1) {
					changed = 1;
					chord->need_recompute_parameters = true;
					//  we change the onset of the first chord
					if (gathering_policy_ms > 0) // align to last
						chord->onset = last_onset;
					else if (gathering_policy_ms == 0) // align to average
						chord->onset = gathering_average_onset;
					// else: nothing to do: it stays aligned to first chord
				}
			}
			
			if (threshold_cents >= 0.) { // pitch merging
				note = chord->firstnote;
				while (note) {
					t_note *note2;
					long count_note = 1; 
					double gathering_average_pitch = note->midicents; 
					double gathering_average_velocity = note->velocity; 
					double last_pitch = note->midicents; 
					double last_velocity = note->velocity; 
					note2 = note->next;
					while (note2 && (note2->midicents - note->midicents <= threshold_cents)) {
						count_note++;
						gathering_average_pitch += note2->midicents;
						gathering_average_velocity += note2->velocity;
						last_pitch = note2->midicents;
						last_velocity = note2->velocity;
						
						// we delete the note
						note_delete((t_notation_obj *)x, note2, false);
						note2 = note2->next;
					}
					gathering_average_pitch /= count_note;
					gathering_average_velocity /= count_note;
					if (count_note > 1) {
						changed = 1;
						chord->need_recompute_parameters = true;
						//  we change the pitch of the first note
						if (gathering_policy_cents > 0) { // align to last pitch
							note->midicents = last_pitch;
							note->velocity = last_velocity;
						}
						else if (gathering_policy_cents == 0) { // align to average pitch
							note->midicents = gathering_average_pitch;
							note->velocity = gathering_average_velocity;
						}
						// else: nothing to do: it stays aligned to first pitch
					}	
					
					note = note->next;
				
				}
			}
		}
	}
    
    // markers!
    if (markers_also) {
        t_marker *marker;
        if (threshold_ms >= 0.) { // time merging
            for (marker = x->r_ob.firstmarker; marker; marker = marker->next) {
                long count = 1;
                double gathering_average_onset = marker->position_ms;
                double last_onset = marker->position_ms;
                t_marker *marker2 = marker->next;
                char merged = false;
                while (marker2 && (marker2->position_ms - marker->position_ms <= threshold_ms)) {
                    if (!only_selected || notation_item_is_globally_selected((t_notation_obj *)x, (t_notation_item *)marker2)) {
                        t_marker *marker_to_merge;
                        count++;
                        gathering_average_onset += marker2->position_ms;
                        last_onset = marker2->position_ms;
                        // we merge the chord to the first one
                        delete_marker((t_notation_obj *)x, marker2);
                        merged = true;
                    }
                    marker2 = marker2->next;
                }
                
                if (merged)
                    check_correct_scheduling((t_notation_obj *)x, false);
                
                gathering_average_onset /= count;
                if (count > 1) {
                    changed = 1;
                    //  we change the onset of the first chord
                    if (gathering_policy_ms > 0) // align to last
                        marker->position_ms = last_onset;
                    else if (gathering_policy_ms == 0) // align to average
                        marker->position_ms = gathering_average_onset;
                    // else: nothing to do: it stays aligned to first chord
                }
            }
        }
    }
    
    if (changed) {
        update_all_accidentals_if_needed(x);
		recompute_total_length((t_notation_obj *)x);
    }
	
	return changed;
}



void roll_merge(t_roll *x, t_symbol *s, long argc, t_atom *argv){

	if (proxy_getinlet((t_object *) x) == 0) {
		t_llll *inputlist = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE); // We clone it: we operate destructively
		if (inputlist && inputlist->l_head) {
			t_llllelem *firstelem = inputlist->l_head;
			double threshold_ms = -1.;  // negative = not computed
			double threshold_cents = -1.;
			char gathering_policy_ms = 0;
			char gathering_policy_cents = 0;
			char only_selected = false;
			
			if (firstelem && hatom_gettype(&firstelem->l_hatom) == H_SYM && hatom_getsym(&firstelem->l_hatom) == _llllobj_sym_selection) {
				only_selected = true;
				llll_behead(inputlist);
				firstelem = inputlist->l_head;
			}
				
			if (firstelem) {
				threshold_ms = hatom_getdouble(&firstelem->l_hatom);
				if (firstelem->l_next) {
					threshold_cents = hatom_getdouble(&firstelem->l_next->l_hatom);
					if (firstelem->l_next->l_next) {
						gathering_policy_ms = hatom_getlong(&firstelem->l_next->l_next->l_hatom);
						if (firstelem->l_next->l_next->l_next) {
							gathering_policy_cents = hatom_getlong(&firstelem->l_next->l_next->l_next->l_hatom);
						}
					}
				}
			}
			
			create_whole_roll_undo_tick(x);

			// ok, ready to merge.
			lock_general_mutex((t_notation_obj *)x);
			merge(x, threshold_ms, threshold_cents, gathering_policy_ms, gathering_policy_cents, only_selected, true);
			unlock_general_mutex((t_notation_obj *)x);

			llll_free(inputlist);

			handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_MERGE);
		}
	}

}

void roll_subroll(t_roll *x, t_symbol *s, long argc, t_atom *argv){
// outputs a "subroll"
// syntax: select (voice1 voice2 ... ) (start_ms end_ms)
// use () for all voices, use () for all durations.
// e.g. select (1 3 4) (100 500) 
//		selects voices 1, 3 and 4 from 100 to 500 ms. Use negative values to say: default values (e.g. (100 -1) means from 100ms to the end)

	if (proxy_getinlet((t_object *) x) == 0) {
		t_llll *inputlist = llllobj_parse_llll((t_object *) x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_CLONE); // We clone it: we operate destructively
		if (inputlist && inputlist->l_head) {
			double from_ms, to_ms;
			t_llll *which_voices;
			t_llll *what_to_dump = NULL;
			char subroll_type = 0; //0 = standard, 1 = keep only notes whose onset is inside the time lapse 
			
			if (inputlist->l_head && hatom_gettype(&inputlist->l_head->l_hatom) == H_SYM && hatom_getsym(&inputlist->l_head->l_hatom) == _llllobj_sym_onset) {
				subroll_type = 1;
				llll_destroyelem(inputlist->l_head);
			}
			
			if (hatom_gettype(&inputlist->l_head->l_hatom) == H_LLLL)
				which_voices = hatom_getllll(&inputlist->l_head->l_hatom);
			else
				which_voices = llll_get(); // all voices

			from_ms = 0; to_ms = x->r_ob.length_ms;
			if (inputlist->l_head->l_next) { // there is a ms specification
				t_llllelem *secondelem = inputlist->l_head->l_next;
				if (hatom_gettype(&secondelem->l_hatom) == H_LLLL) {
					t_llll *boundaries_ms = hatom_getllll(&secondelem->l_hatom);
					if (boundaries_ms->l_size >=2) {
						from_ms = hatom_getdouble(&boundaries_ms->l_head->l_hatom);
						to_ms = hatom_getdouble(&boundaries_ms->l_head->l_next->l_hatom);
						if (from_ms < 0) from_ms = 0.;
						if (to_ms < 0) to_ms = x->r_ob.length_ms;
					}
				}
				if (secondelem && secondelem->l_next && hatom_gettype(&secondelem->l_next->l_hatom) == H_LLLL)
					what_to_dump = hatom_getllll(&secondelem->l_next->l_hatom);
			}
		
			send_subroll_values_as_llll(x, which_voices, from_ms, to_ms, what_to_dump, subroll_type);

			llll_free(inputlist);
		}
	}
}

/*
char delete_selection(t_roll *x, char only_editable_items){ // the longs are the region boundaries (milliseconds and midicents)
//delete chords/notes within the selected region
	t_notation_item *curr_it = NULL, *nextitem = NULL;
	char changed = 0;
	t_notation_item *lambda_it = x->r_ob.lambda_selected_item_ID > 0 ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : NULL;
	
	lock_general_mutex((t_notation_obj *)x);	
	for (curr_it = x->r_ob.firstselecteditem; curr_it; curr_it = nextitem) { // cycle on the selected items
		nextitem = curr_it->next_selected; 
		
		if (lambda_it && (lambda_it == curr_it || // lambda item is exactly the item we're deleting..
						  notation_item_is_ancestor_of((t_notation_obj *)x, lambda_it, curr_it) || // or one of its ancestors...
						  notation_item_is_ancestor_of((t_notation_obj *)x, curr_it, lambda_it))) { // or one of its progeny...
			cpost("Trying to delete item %p (type %ld). Can't.", curr_it, curr_it->type);
			object_error((t_object *)x, "Can't delete item, it's being output from the playout!");
			continue;
		}
		
		if (curr_it->type == k_NOTE && (!only_editable_items || is_editable((t_notation_obj *)x, ((t_note *) curr_it)->parent->num_notes == 1 ? k_CHORD : k_NOTE, k_DELETION))) { // it is a note
			t_note *nt = (t_note *) curr_it;
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)){
                t_rollvoice *voice = nt->parent->voiceparent;
				create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)nt->parent, k_CHORD, nt->parent->num_notes == 1 ? k_UNDO_MODIFICATION_ADD : k_UNDO_MODIFICATION_CHANGE);
				note_delete((t_notation_obj *)x, (t_note *)curr_it, false);
                update_all_accidentals_for_voice_if_needed(x, voice);
				changed = 1;
			}
		} else if (curr_it->type == k_CHORD && (!only_editable_items || is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_DELETION))) {
			t_chord *ch = ((t_chord *)curr_it);
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)ch)) {
				t_note *nt = ch->firstnote, *nt2;
                t_rollvoice *voice = ch->voiceparent;
				create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_CHORD, k_UNDO_MODIFICATION_ADD);
				while (nt) {
					nt2 = nt->next;
					if (!nt->locked){
						note_delete((t_notation_obj *)x, nt, false);
						changed = 1;
					} else {
						ch->locked = true; //something remains in the chord (locked notes: chords is locked!)
					}
					nt = nt2;
				}
                update_all_accidentals_for_voice_if_needed(x, voice);
			}
		} else if (curr_it->type == k_LYRICS && (!only_editable_items || is_editable((t_notation_obj *)x, k_LYRICS, k_DELETION))) {
			t_lyrics *ly = (t_lyrics *)curr_it;
			create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ly->owner, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
			changed |= delete_chord_lyrics((t_notation_obj *) x, (t_chord *) ly->owner);

		} else if (curr_it->type == k_MARKER && (!only_editable_items || is_editable((t_notation_obj *)x, k_MARKER, k_DELETION))) {
			create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
			delete_marker((t_notation_obj *) x, (t_marker *) curr_it);
			changed = true;
			update_hscrollbar((t_notation_obj *)x, 0);
		}
	}
	
	clear_selection((t_notation_obj *) x);
	
    if (changed) {
		recompute_total_length((t_notation_obj *)x);
        update_hscrollbar((t_notation_obj *)x, x->r_ob.lambda_spacing != k_CUSTOMSPACING_NONE ? 1 : 2);
        invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
    }
	
	unlock_general_mutex((t_notation_obj *)x);
	
	check_correct_scheduling((t_notation_obj *)x, true);
	
	return changed;
}



char ripple_delete_selection(t_roll *x, char only_editable_items)
{
    double left_ms = get_selection_leftmost_onset(x);
    double right_ms = get_selection_rightmost_onset(x);
    double delta = left_ms - right_ms;
    char res = delete_selection(x, true);
    preselect_elements_in_region_for_mouse_selection(x, right_ms, x->r_ob.length_ms + 1000, -320000, 36000, 0, x->r_ob.num_voices);
    preselect_markers_in_region((t_notation_obj *)x, right_ms, x->r_ob.length_ms + 1000);
    move_preselecteditems_to_selection((t_notation_obj *)x, k_SELECTION_MODE_FORCE_SELECT, false, false);
    change_selection_onset(x, &delta);
    clear_selection((t_notation_obj *)x);
    return res;
}
 */

char change_note_voice_from_lexpr_or_llll(t_roll *x, t_note *note, t_lexpr *lexpr, t_llll *new_voice, char also_select){
	char changed = 0;
	t_llllelem *thiselem = new_voice ? new_voice->l_head : NULL;
	
	if (note->parent->num_notes == 1)
		return change_chord_voice_from_lexpr_or_llll(x, note->parent, lexpr, new_voice, also_select);
		
	if ((lexpr || thiselem) && !notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)note)) {
		long new_voice_num = note->parent->voiceparent->v_ob.number + 1;
		change_long((t_notation_obj *)x, &new_voice_num, lexpr, thiselem, false, (t_notation_item *)note);
		t_rollvoice *voice = nth_rollvoice(x, new_voice_num - 1);
		if (voice) {
			create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)note->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
			
			t_note *cloned = clone_note((t_notation_obj *)x, note, k_CLONE_FOR_ORIGINAL);
			t_chord *new_chord = build_chord_from_notes((t_notation_obj *)x, cloned, cloned);
			new_chord->onset = note->parent->onset;
			insert_chord(x, voice, new_chord, 0);
			note_delete((t_notation_obj *)x, note, false);
            update_all_accidentals_for_voice_if_needed(x, voice);
			
			if (also_select)
				notation_item_add_to_preselection((t_notation_obj *)x, (t_notation_item *)new_chord);
			
			create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)new_chord, k_CHORD, k_UNDO_MODIFICATION_DELETE);
			
			new_chord->need_recompute_parameters = true;
			set_need_perform_analysis_and_change_flag((t_notation_obj *)x);
			
			changed = 1;
		}
	}
	
	return changed;
}

char change_chord_voice_from_lexpr_or_llll(t_roll *x, t_chord *chord, t_lexpr *lexpr, t_llll *new_voice, char also_select){
	char changed = 0;
	t_llllelem *thiselem = new_voice ? new_voice->l_head : NULL;
	
	if ((lexpr || thiselem) && !notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)chord)) {
		long new_voice_num = chord->voiceparent->v_ob.number + 1;
		change_long((t_notation_obj *)x, &new_voice_num, lexpr, thiselem, false, (t_notation_item *)chord);
		t_rollvoice *new_voice = nth_rollvoice(x, new_voice_num - 1);
		if (new_voice) {
			create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)chord, k_CHORD, k_UNDO_MODIFICATION_ADD);
			
			t_chord *cloned = clone_chord((t_notation_obj *)x, chord, k_CLONE_FOR_ORIGINAL);
			delete_chord_from_voice((t_notation_obj *)x, chord, NULL, false);
			insert_chord(x, new_voice, cloned, 0);
			
			if (also_select)
				notation_item_add_to_preselection((t_notation_obj *)x, (t_notation_item *)cloned);

			create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)cloned, k_CHORD, k_UNDO_MODIFICATION_DELETE);
			
			cloned->need_recompute_parameters = true;
			set_need_perform_analysis_and_change_flag((t_notation_obj *)x);
			
			changed = 1;
		}
	}
		
	return changed;
}



t_chord *get_nearest_chord(t_roll *x, double ms)
{
	t_rollvoice *voice;
	t_chord *chord, *best_chord = NULL;
	double best_diff = 0;
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
		for (chord = voice->firstchord; chord; chord = chord->next) {
			double this_diff = fabs(chord->onset - ms);
			if (!best_chord || this_diff < best_diff) {
				best_chord = chord;
				best_diff = this_diff;
			} else if (chord->onset > ms)
				break;
		}
	}
	return best_chord;
}


t_atom_long roll_acceptsdrag_unlocked(t_roll *x, t_object *drag, t_object *view)
{
    return notationobj_acceptsdrag((t_notation_obj *)x, drag, view);
}


long roll_oksize(t_roll *x, t_rect *newrect)
{
//	post("OkSize: size %.2f, %.2f, it's me? %d", newrect->width, newrect->height, x->r_ob.itsme);
	if (!x->r_ob.itsme) {
		if (x->r_ob.link_vzoom_to_height)
			notationobj_set_vzoom_depending_on_height((t_notation_obj *) x, newrect->height);
		else {
			notationobj_reset_size_related_stuff((t_notation_obj *) x);
			x->r_ob.needed_uheight = notationobj_get_supposed_standard_height((t_notation_obj *)x);
			x->r_ob.needed_uheight_for_one_system = x->r_ob.needed_uheight / ((x->r_ob.num_systems > 0) ? x->r_ob.num_systems : 1);
		}
        
        x->r_ob.inner_width = newrect->width - (2 * x->r_ob.j_inset_x);

        redraw_hscrollbar((t_notation_obj *) x, 1); // x->r_ob.vscrollbar_pos
		redraw_vscrollbar((t_notation_obj *) x, 1);
		calculate_ms_on_a_line((t_notation_obj *) x);
		recalculate_num_systems((t_notation_obj *)x);
		x->r_ob.system_jump = get_system_jump((t_notation_obj *)x);
		x->r_ob.firsttime = true;
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
		return 1;
	}
	return 0;
}

void roll_deletechord(t_roll *x, t_symbol *s, long argc, t_atom *argv){
// delete a chord with a given noteID, unused
}

	
t_roll* roll_new(t_symbol *s, long argc, t_atom *argv)
{
	int i;
	t_roll* x = NULL;
	t_max_err err = MAX_ERR_GENERIC;
	t_dictionary *d;
	long flags;
	int v; 
	t_rollvoice *voiceprec = NULL;

    if (!(d=object_dictionaryarg(argc,argv)))
		return NULL;

	x = (t_roll*) object_alloc_debug(s_roll_class);
	flags = 0 
			| JBOX_DRAWFIRSTIN 
			| JBOX_NODRAWBOX
			| JBOX_DRAWINLAST
	//		| JBOX_TRANSPARENT	
	//		| JBOX_NOGROW
	//		| JBOX_GROWY
			| JBOX_GROWBOTH
			| JBOX_HILITE
	//		| JBOX_BACKGROUND
			| JBOX_TEXTFIELD
	//		| JBOX_DRAWBACKGROUND
	//		| JBOX_DEFAULTNAMES
	//		| JBOX_MOUSEDRAGDELTA
			;
	
	x->r_ob.creatingnewobj = 1;
	err = jbox_new(&x->r_ob.j_box.l_box, flags, argc, argv); 
	x->r_ob.j_box.l_box.b_firstin = (t_object*) x;

	x->r_ob.obj_type = k_NOTATION_OBJECT_ROLL;
	
	initialize_notation_obj((t_notation_obj *) x, k_NOTATION_OBJECT_ROLL, (rebuild_fn) set_roll_from_llll, (notation_obj_fn) create_whole_roll_undo_tick, 
							(notation_obj_notation_item_fn) force_notation_item_inscreen);

	roll_declare_bach_attributes(x);
	
    x->r_ob.inner_width = 526 - (2 * x->r_ob.j_inset_x); // 526 is the default object width

    x->r_ob.show_page_numbers = 1;
	x->r_ob.page_number_offset = 0; 
	x->r_ob.current_page = 0;
	x->r_ob.need_vscrollbar = 0;
	x->must_append_chords = false;
	x->must_preselect_appended_chords = false;
	x->pasting_chords = false;
 	x->must_apply_delta_onset = 0.;
	x->r_ob.accidentals_preferences = k_ACC_AUTO;
	
	// retrieving patcher parent
	object_obex_lookup(x, gensym("#P"), &(x->r_ob.patcher_parent));
	
	// initializing old undo/redo lists
	for (i = 0; i < CONST_MAX_UNDO_STEPS; i++) {
		x->r_ob.old_undo_llll[i] = NULL;
		x->r_ob.old_redo_llll[i] = NULL;
	}
	
	// initializing all rollvoices (we DON'T fill them, but we have them). 
	x->r_ob.voiceuspacing_as_floatlist[0] = 0.;
	for (v = 0; v < CONST_MAX_VOICES; v++) {
		t_rollvoice *voice = (t_rollvoice *)bach_newptr(sizeof(t_rollvoice));
		if (v == 0) {
			x->firstvoice = voice;
			x->r_ob.firstvoice = (t_voice *)voice;
		}
		if (v == CONST_MAX_VOICES - 1) {
			x->lastvoice = voice;
			x->r_ob.lastvoice = (t_voice *)voice;
		}
		initialize_rollvoice((t_notation_obj *)x, voice, v, gensym("CM"));
		
		voice->next = NULL; 
		voice->prev = voiceprec;
		if (voiceprec)
			voiceprec->next = voice; 
		voiceprec = voice;
	}

	// no screen has been painted yet: this depends on the view! 
	x->r_ob.domain = DBL_MIN; // needed for initialization
	x->r_ob.screen_ms_start = 0;
	x->r_ob.screen_ms_end = 0;
	x->r_ob.inner_width = -1;
	x->r_ob.inner_height = -1;
	x->r_ob.length_ms = 2000;
    x->r_ob.length_ux = 200; // xposition_to_unscaled_xposition((t_notation_obj *)x, onset_to_xposition((t_notation_obj *)x, x->r_ob.length_ms, NULL));

	x->r_ob.first_shown_system = 0;
	x->r_ob.last_shown_system = 0;

	initialize_slots((t_notation_obj *) x, false);
	initialize_commands((t_notation_obj *) x);
	initialize_popup_menus((t_notation_obj *) x);
	
	
    // @arg 0 @name numvoices @optional 1 @type int @digest Number of voices
	notation_obj_arg_attr_dictionary_process_with_bw_compatibility(x, d);
	
	t_llll *right = llll_slice(x->r_ob.voicenames_as_llll, x->r_ob.num_voices);
	llll_free(right);

	right = llll_slice(x->r_ob.stafflines_as_llll, x->r_ob.num_voices);
	llll_free(right);
		
	parse_fullaccpattern_to_voices((t_notation_obj *) x);

	// setup llll
	llllobj_jbox_setup((t_llllobj_jbox *) x, 6, "b4444444"); 

	initialize_textfield((t_notation_obj *) x);	

	// clock
	x->r_ob.m_clock = clock_new_debug((t_object *)x, (method) roll_task);

	// retrieving vlues
	x->r_ob.add_staff = 0;
	x->r_ob.itsme = 0;
	x->r_ob.add_voice = 0;
	x->r_ob.step_y = CONST_STEP_UY;	
	x->r_ob.zoom_x = x->r_ob.horizontal_zoom / 100.;
	x->r_ob.j_inset_y = 0.5;
    set_mousedown((t_notation_obj *)x, NULL, k_NONE);

	lock_general_mutex((t_notation_obj *)x);
	clear_selection((t_notation_obj *) x);
	clear_preselection((t_notation_obj *) x);
	unlock_general_mutex((t_notation_obj *)x);
	
	x->r_ob.active_slot_num = -1; // active slot is the normal view (= no slot is visible!)
    x->r_ob.active_slot_num_1based = 0;
	x->r_ob.m_editor = NULL;
	x->r_ob.play_head_ms = -1;

	x->r_ob.key_signature_uwidth = get_max_key_uwidth((t_notation_obj *) x);
	calculate_ms_on_a_line((t_notation_obj *) x);
	recalculate_num_systems((t_notation_obj *)x);
	x->r_ob.system_jump = get_system_jump((t_notation_obj *)x);
	
	// creating proxies
	x->m_proxy5 = proxy_new_debug((t_object *) x, 5, &x->m_in);
	x->m_proxy4 = proxy_new_debug((t_object *) x, 4, &x->m_in);
	x->m_proxy3 = proxy_new_debug((t_object *) x, 3, &x->m_in);
	x->m_proxy2 = proxy_new_debug((t_object *) x, 2, &x->m_in);
	x->m_proxy1 = proxy_new_debug((t_object *) x, 1, &x->m_in);
	
	object_attach_byptr_register(x, x, CLASS_BOX);

	jbox_ready(&x->r_ob.j_box.l_box);

	if (x) {
		// retrieving values
		t_llll *llll_for_rebuild = llll_retrieve_from_dictionary(d, "whole_roll_data");
		if (llll_for_rebuild) { // new method
			llllobj_manage_dict_llll((t_object *)x, LLLL_OBJ_UI, llll_for_rebuild);
            
			set_roll_from_llll(x, llll_for_rebuild, true);
			if (x->r_ob.send_rebuild_done_at_startup)
				handle_rebuild_done((t_notation_obj *) x);
			llll_free(llll_for_rebuild);
		} else { // old method
			long ac; t_atom *av = NULL;
			t_max_err err = dictionary_getatoms(d, gensym("whole_roll_data"), &ac, &av); 
			if (!err && ac && av)
				roll_anything(x, NULL, ac, av);
		}
		
		if (!USE_NEW_UNDO_SYSTEM && x->r_ob.allow_undo)
			x->r_ob.old_undo_llll[0] = get_roll_values_as_llll(x, k_CONSIDER_FOR_SAVING, 
															   (e_header_elems) (k_HEADER_BODY | k_HEADER_SLOTINFO | k_HEADER_VOICENAMES | k_HEADER_MARKERS | k_HEADER_GROUPS  | k_HEADER_ARTICULATIONINFO | k_HEADER_NOTEHEADINFO | k_HEADER_NUMPARTS), true, false);

		if (x->r_ob.automessage_ac > 0)
			x->r_ob.need_send_automessage = true;
		
		x->r_ob.last_undo_time = systime_ms();
        
        
        // N.B.: The version_number attribute is actually EXTREMELY useful: when an object is created in Max its dictionary has 0 as its default value when the
        // new() method is called for the first time, and something > 0 when e.g. it was already saved.
        x->r_ob.version_number = BACH_CURRENT_VERSION;
        x->r_ob.creatingnewobj = 0;

        return x;
	}
    

	object_free_debug(x); // unlike freeobject(), this works even if the argument is NULL
	return NULL; 

}

void roll_int(t_roll *x, t_atom_long num){
	t_atom argv[1]; atom_setlong(argv, num);
	roll_anything(x, _llllobj_sym_list, 1, argv);
}

void roll_float(t_roll *x, double num){
	t_atom argv[1]; atom_setfloat(argv, num);
	roll_anything(x, _llllobj_sym_list, 1, argv);
}

void roll_free(t_roll *x){
	t_rollvoice *voice, *temp;
	
	// deleting all chord datas
	clear_roll_body(x, -2);
	
	free_notation_obj((t_notation_obj *) x);
	
	// freeing proxies
	object_free_debug(x->m_proxy1);
	object_free_debug(x->m_proxy2);
	object_free_debug(x->m_proxy3);
	object_free_debug(x->m_proxy4);
	object_free_debug(x->m_proxy5);
	
	object_free_debug(x->r_ob.m_clock);
	
	// freeing all voices
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < CONST_MAX_VOICES)) {
		temp = voice->next;
		free_voice((t_notation_obj *)x, (t_voice *)voice);
		voice = temp;
	}
}

void check_all_chords_order_and_correct_scheduling_fn(t_bach_inspector_manager *man, void *obj, t_bach_attribute *attr)
{
    t_notation_obj *r_ob = (man->bach_managing ? (t_notation_obj *)man->owner : NULL);
    if (r_ob->obj_type == k_NOTATION_OBJECT_ROLL) {
        t_roll *x = (t_roll *)r_ob;
        check_all_chords_order(x);
        check_correct_scheduling((t_notation_obj *)x, true);
        recompute_total_length((t_notation_obj *)x);
        update_hscrollbar((t_notation_obj *)x, 0);
    }
}

void check_all_chords_order(t_roll *x){
	t_rollvoice *voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		if (voice->firstchord) {
			check_chords_order_for_voice(x, voice);
		}
		voice = voice->next;
	}
}

void check_all_chords_and_notes_order(t_roll *x){
	t_rollvoice *voice = x->firstvoice;
	t_chord *ch;
	check_all_chords_order(x);
	for (voice = x->firstvoice; voice && (voice->v_ob.number < x->r_ob.num_voices); voice = voice->next)
		for (ch = voice->firstchord; ch; ch = ch->next)
			check_notes_order(ch);
}


// functions to check the order of the elements in the linked list. Returns 0 if the order was already correct, 1 if changed.
// check_notes_order is performed inside calculate_chord_parameters. check_chords_order_for_voice is performed each time a chord is moved.
char check_chords_order_for_voice(t_roll *x, t_rollvoice *voice){
	char changed = false;
	t_chord *curr_ch = voice->firstchord;
	
	char swapped;
	do {
		swapped = false;
		for (curr_ch = voice->firstchord; curr_ch && curr_ch->next; curr_ch = curr_ch->next) {
			if (curr_ch->next->onset < curr_ch->onset){ // we have to swap them
				swapped = true;
				if (curr_ch->prev){
					if (curr_ch->next->next) { // swap between two inner chords
						//					post("inner swap");
						t_chord *temp1, *temp2, *temp3, *temp4;
						temp1 = curr_ch->prev; temp2=curr_ch; temp3=curr_ch->next; temp4=curr_ch->next->next;
						curr_ch->next->next->prev = temp2;
						curr_ch->prev->next = temp3;
						curr_ch->next->next = temp2;
						curr_ch->next->prev = temp1;
						curr_ch->next = temp4;
						curr_ch->prev = temp3;
						changed = true;
					} else { // swap between the last two chords
						//					post("last swap");
						t_chord *temp1, *temp2, *temp3, *temp4;
						temp1 = curr_ch->prev; temp2=curr_ch; temp3=curr_ch->next; temp4=NULL;
						//					post("temp1: %lx, temp2: %lx, temp3: %lx, temp4: %lx", temp1, temp2, temp3, temp4);
						curr_ch->prev->next = temp3;
						curr_ch->next->next = temp2;
						curr_ch->next->prev = temp1;
						curr_ch->next = NULL;
						curr_ch->prev = temp3;
						voice->lastchord = temp2;
						changed = true;
						//					break; // if we have swapped the last two chords, we're done.
					}
				} else { // swap between 1st and 2nd chords
					//				post("first swap");
					t_chord *temp1, *temp2, *temp3, *temp4;
					temp1 = NULL; temp2=curr_ch; temp3=curr_ch->next; temp4=curr_ch->next->next;
					if (!(temp4)) // we have just 2 chords:
						voice->lastchord = temp2;
					else
						curr_ch->next->next->prev = temp2;
					curr_ch->next->next = temp2;
					curr_ch->next->prev = NULL;
					curr_ch->next = temp4;
					curr_ch->prev = temp3;
					voice->firstchord = temp3;
					changed = true;
				}
			}
		}
	} while (swapped);
	
	return changed;
}


char chord_get_screen_position_for_painting(t_roll *x, t_chord *curr_ch)
{
	double max_note_duration = chord_get_max_duration((t_notation_obj *) x, curr_ch);
	double startpad_ms = x->r_ob.additional_ux_start_pad == 0 ? 0 : deltaxpixels_to_deltaonset((t_notation_obj *) x, x->r_ob.additional_ux_start_pad * x->r_ob.zoom_x);
    
    switch (x->r_ob.view) {
        case k_VIEW_SCROLL:
            if (curr_ch->onset + max_note_duration < x->r_ob.screen_ms_start - startpad_ms - 200 / x->r_ob.zoom_x)
                return -1;
            if (curr_ch->onset > x->r_ob.screen_ms_end)
                return 1;
            return 0;
            
        case k_VIEW_PAPYRUS:
            if (curr_ch->system_index < x->r_ob.first_shown_system)
                return -1;
            if (curr_ch->system_index > x->r_ob.last_shown_system)
                return 1;
            return 0;
            
        case k_VIEW_PAGE:
            if (curr_ch->system_index < x->r_ob.first_shown_system)
                return -1;
            if (curr_ch->system_index <= x->r_ob.last_shown_system)
                return 1;
            return 0;

        default:
            return 0;
    }
}

char is_notehead_inscreen_for_painting(t_roll *x, t_note *curr_nt){
	t_chord *curr_ch = curr_nt->parent;
	double startpad_ms = x->r_ob.additional_ux_start_pad == 0 ? 0 : deltaxpixels_to_deltaonset((t_notation_obj *) x, x->r_ob.additional_ux_start_pad * x->r_ob.zoom_x);
	if ((x->r_ob.view == k_VIEW_SCROLL && (curr_ch->onset + curr_nt->duration >= x->r_ob.screen_ms_start - startpad_ms - 300 / x->r_ob.zoom_x) && 
							 (curr_ch->onset - 20 / x->r_ob.zoom_x <= x->r_ob.screen_ms_end)) ||
							(x->r_ob.view != k_VIEW_SCROLL))
		return true;
	return false;
}

void paint_static_stuff1(t_roll *x, t_object *view, t_rect rect, t_jfont *jf, t_jfont *jf_acc, t_jfont *jf_text_fractions, t_jfont *jf_acc_bogus)
{
	
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("static_layer1"), rect.width, rect.height);

	if (g){
		t_jfont *jf_text_small, *jf_text_smallbold, *jf_text_markers, *jf_lyrics, *jf_lyrics_nozoom, *jf_ann, *jf_dynamics, *jf_small_dynamics, *jf_dynamics_nozoom;
		t_rollvoice *voice;
		double playhead_y1, playhead_y2;
		double system_jump = x->r_ob.system_jump;
		double octave_stem_length = 7 * x->r_ob.step_y;
/*		double end_x_to_repaint_no_inset = unscaled_xposition_to_xposition((t_notation_obj *) x, x->r_ob.screen_ux_start - CONST_X_LEFT_START_DELETE_UX_ROLL) - x->r_ob.additional_ux_start_pad * x->r_ob.zoom_y;
*/
        double end_x_to_repaint_no_inset = (22 + x->r_ob.key_signature_uwidth + x->r_ob.voice_names_uwidth + x->r_ob.additional_ux_start_pad) * x->r_ob.zoom_y - x->r_ob.additional_ux_start_pad * x->r_ob.zoom_y + x->r_ob.j_inset_x;

		
		// some constant that will be useful later for the "retouches" left to do, in order to have things working properly
		// e.g.: if some notes have been drawn over these parts, we cover the notes with the keys/background/staves...
        
//		end_x_to_repaint_no_inset = onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_start - CONST_X_LEFT_START_DELETE_MS / x->r_ob.zoom_x, NULL) - x->r_ob.additional_ux_start_pad * x->r_ob.zoom_y;
		
		// defining fonts
		jf_text_small = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, round(x->r_ob.slot_background_font_size * x->r_ob.zoom_y * (x->r_ob.bgslot_zoom/100.)));  // text font (small)
		jf_text_smallbold = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD, round(x->r_ob.slot_background_font_size * x->r_ob.zoom_y * (x->r_ob.bgslot_zoom/100.)));  // text font (small and bold)
		jf_text_markers = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD, x->r_ob.markers_font_size * x->r_ob.zoom_y);  // text font for markers
		jf_lyrics = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, x->r_ob.lyrics_font_size * x->r_ob.zoom_y);
		jf_lyrics_nozoom = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, x->r_ob.lyrics_font_size);
        jf_ann = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, x->r_ob.annotation_font_size * x->r_ob.zoom_y);
        jf_small_dynamics = jfont_create_debug("November for bach", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD, x->r_ob.slot_background_font_size * 2 * x->r_ob.zoom_y * (x->r_ob.bgslot_zoom/100.));
        jf_dynamics = jfont_create_debug("November for bach", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD, x->r_ob.dynamics_font_size * x->r_ob.zoom_y);
        jf_dynamics_nozoom = jfont_create_debug("November for bach", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD, x->r_ob.dynamics_font_size);
		
		get_playhead_ypos((t_notation_obj *)x, rect, &playhead_y1, &playhead_y2);
		
		lock_general_mutex((t_notation_obj *)x);
		for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) { // cycle on the voices
			t_jrgba mainstaffcolor = get_mainstaff_color((t_notation_obj *) x, voice->v_ob.r_it.selected, voice->v_ob.locked, voice->v_ob.muted, voice->v_ob.solo);
			t_jrgba auxstaffcolor = get_auxstaff_color((t_notation_obj *) x, voice->v_ob.r_it.selected, voice->v_ob.locked, voice->v_ob.muted, voice->v_ob.solo);
			t_jrgba notecolor, tailcolor, stemcolor, accidentalcolor, durationlinecolor;
			t_chord *curr_ch;
			double staff_bottom_y, staff_top_y;
			long k; 
			long clef = get_voice_clef((t_notation_obj *)x, (t_voice *)voice);
			char lyrics_dashed_going_on = 0;
			double left_dashed_x = 0.;
            char voice_linear_edited = (x->r_ob.notation_cursor.voice == (t_voice *)voice);
            char is_in_voiceensemble = (voiceensemble_get_numparts((t_notation_obj *)x, (t_voice *)voice) > 1);
            char part_direction = is_in_voiceensemble ? (voice->v_ob.part_index % 2 == 1 ? -1 : 1) : 0;

            double curr_hairpin_start_x = -100;
            long curr_hairpin_type = 0;
            char first_painted_chord = true;
            t_jrgba prev_hairpin_color = x->r_ob.j_dynamics_rgba;
            char prev_hairpin_dontpaint = false;
            
			if (voice->v_ob.hidden) // voice is hidden!
				continue;
			
			// Boundary line between voices: needed for debug
			//		if (voice->prev)
			//			paint_line((t_notation_obj *) x, g, x->r_ob.j_selection_rgba, 0, voice->v_ob.offset_y + ((CONST_DEFAULT_ROLLVOICES_SPACING_UY - voice->prev->v_ob.vertical_spacing) / 2. + CONST_VOICE_THRESHOLD) *  x->r_ob.zoom_y, rect.width, voice->v_ob.offset_y + ((CONST_DEFAULT_ROLLVOICES_SPACING_UY - voice->prev->v_ob.vertical_spacing) / 2 + CONST_VOICE_THRESHOLD) * x->r_ob.zoom_y, 1.);
			//	
			compute_middleC_position_for_voice((t_notation_obj *) x, (t_voice *) voice);
			staff_bottom_y = get_staff_bottom_y((t_notation_obj *) x, (t_voice *) voice, false);
			staff_top_y = get_staff_top_y((t_notation_obj *) x, (t_voice *) voice, false);
			
            if (voice_linear_edited) {
                double x1 = onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_start, NULL);
                double x2 = onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_end, NULL);
                paint_rectangle(g, get_grey(1.), change_alpha(x->r_ob.j_linear_edit_rgba, 0.2), x1, staff_top_y, x2 - x1, staff_bottom_y - staff_top_y, 0.);
            }
            
			// paint staff lines
            if (voice->v_ob.part_index == 0)
                for (k=x->r_ob.first_shown_system; k <= x->r_ob.last_shown_system; k++)
                    paint_staff_lines((t_notation_obj *)x, g, end_x_to_repaint_no_inset, rect.width - x->r_ob.j_inset_x, 1.,
								  voice->v_ob.middleC_y + k * system_jump, clef, mainstaffcolor, auxstaffcolor, voice->v_ob.num_staff_lines, voice->v_ob.staff_lines);
			
			// clefs later! at the end!

#ifdef BACH_PAINT_IDS
			char text[20];
			snprintf_zero(text, 40, "%ld", voice->v_ob.r_it.ID);
			write_text(g, jf_text_markers, build_jrgba(0, 0.3, 0, 1), text, x->r_ob.j_inset_x + 20 * x->r_ob.zoom_y, staff_top_y - 40, rect.width, 40, JGRAPHICS_TEXT_JUSTIFICATION_LEFT + JGRAPHICS_TEXT_JUSTIFICATION_BOTTOM, true, false);
#endif
			
			// chords and notes!
			for (curr_ch = voice->firstchord; curr_ch; curr_ch = curr_ch->next) { // cycle on the chords
				long system_index = 0; // index of system
				double system_shift = 0; // vertical shift due to the system index 
				
				if (!curr_ch->firstnote) {
					object_warn((t_object *) x, "Error: chord with 0 notes detected!");
					continue;
				}
				
				curr_ch->system_index = system_index = onset_to_system_index((t_notation_obj *) x, curr_ch->onset);
				
				// we check if the chord is IN the screen: if so, we draw it!
                char position = chord_get_screen_position_for_painting(x, curr_ch);
                
				if (position == 0) { // inside the screen
					double chord_alignment_x, stem_x;
					char is_chord_selected, is_chord_locked, is_chord_muted, is_chord_solo, is_chord_linear_edited;
					char is_chord_played;
					long num_notes = curr_ch->num_notes;
					double first_note_y_real, last_note_y_real;
					t_note *curr_nt;
					
                    if (first_painted_chord) {
                        long s = x->r_ob.link_dynamics_to_slot - 1;
                        if (x->r_ob.show_hairpins && s >= 0 && s < CONST_MAX_SLOTS && x->r_ob.slotinfo[s].slot_type == k_SLOT_TYPE_DYNAMICS) {
                            // check if there's an hairpin ending on this chord
                            for (t_chord *temp = get_prev_chord(curr_ch); temp; temp = get_prev_chord(temp)) {
                                if (parse_chord_dynamics_easy((t_notation_obj *)x, temp, s, NULL, &curr_hairpin_type)) {
                                    curr_hairpin_start_x = onset_to_xposition((t_notation_obj *) x, temp->onset, NULL);
                                    break;
                                }
                            }
                        }
                        first_painted_chord = false;
                    }
                    
					// finding stem position
					if (x->r_ob.view == k_VIEW_SCROLL) {
						chord_alignment_x = onset_to_xposition((t_notation_obj *) x, curr_ch->onset, NULL);
					} else if (x->r_ob.view == k_VIEW_PAPYRUS) {
						chord_alignment_x = onset_to_xposition((t_notation_obj *) x, curr_ch->onset, &system_index);
						system_shift = system_jump * system_index;
					} else {
						chord_alignment_x = onset_to_xposition((t_notation_obj *) x, curr_ch->onset, NULL);
					}
					
#ifdef BACH_PAINT_IDS
					char text[140];
					snprintf_zero(text, 140, "%ld", curr_ch->r_it.ID);
					write_text(g, jf_text_markers, build_jrgba(0.3, 0, 0, 1), text, curr_ch->stem_x, 
							   staff_top_y - 40, rect.width, 40, JGRAPHICS_TEXT_JUSTIFICATION_LEFT + JGRAPHICS_TEXT_JUSTIFICATION_BOTTOM, true, false);
#endif
					
					// do we have to recalculate chord parameters?
					if (curr_ch->need_recompute_parameters) { // we have to recalculate chord parameters 
						assign_chord_lyrics((t_notation_obj *) x, curr_ch, jf_lyrics_nozoom);
                        assign_chord_dynamics((t_notation_obj *) x, curr_ch, jf_dynamics_nozoom);
						calculate_chord_parameters((t_notation_obj *) x, curr_ch, clef, true);
						curr_ch->need_recompute_parameters = false;
					}

					stem_x = get_stem_x_from_alignment_point_x((t_notation_obj *)x, curr_ch, chord_alignment_x);
					curr_ch->stem_x = stem_x;
					
					// handling selection
					is_chord_selected = (notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)curr_ch) ^ notation_item_is_preselected((t_notation_obj *) x, (t_notation_item *)curr_ch));
                    is_chord_linear_edited = (x->r_ob.notation_cursor.voice && x->r_ob.notation_cursor.chord == curr_ch);
					
					is_chord_locked = notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_ch);
					is_chord_muted = notation_item_is_globally_muted((t_notation_obj *)x, (t_notation_item *)curr_ch); 
					is_chord_solo = notation_item_is_globally_solo((t_notation_obj *)x, (t_notation_item *)curr_ch); 
					is_chord_played = x->r_ob.highlight_played_notes ? curr_ch->played : false; 
					
					t_note *note_1 = nth_note(curr_ch, 0);
					t_note *note_2 = nth_note(curr_ch, num_notes - 1);
					first_note_y_real = note_1 ? system_shift + mc_to_yposition_in_scale((t_notation_obj *) x, note_get_screen_midicents(note_1), (t_voice *) voice) : 0;
					last_note_y_real = note_2 ? system_shift + mc_to_yposition_in_scale((t_notation_obj *) x, note_get_screen_midicents(note_2), (t_voice *) voice) : 0;
					
					// draw stem
					stemcolor = get_stem_color((t_notation_obj *) x, curr_ch, is_chord_selected, is_chord_played, is_chord_locked, is_chord_muted, is_chord_solo, is_chord_linear_edited);
					if (x->r_ob.show_stems > 0) {
						if (curr_ch->direction == 1) { // stem upwards
							paint_line(g, stemcolor, stem_x, first_note_y_real, stem_x, last_note_y_real - octave_stem_length, CONST_STEM_WIDTH);
						} else if (curr_ch->direction == -1) { // stem downwards
							paint_line(g, stemcolor, stem_x, last_note_y_real, stem_x, first_note_y_real + octave_stem_length, CONST_STEM_WIDTH);
						}
					}
					curr_ch->stem_x = stem_x;
					
					double label_family_chord_shape_radius = CONST_LABEL_FAMILY_NOTE_STARTING_URADIUS * x->r_ob.zoom_y;
					
					// we draw the notes and the accidentals
					for (curr_nt = curr_ch->firstnote; curr_nt; curr_nt = curr_nt->next) { // cycle on the notes
						// is the note IN the screen?
						if (is_notehead_inscreen_for_painting(x, curr_nt)) {
							
							// selection (or preselection): 
							char is_note_preselected = notation_item_is_preselected((t_notation_obj *) x, (t_notation_item *)curr_nt);
							char is_note_selected = (notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)curr_nt) ^ is_note_preselected);
							char is_durationline_selected = is_note_selected; 
							char is_tail_selected = (notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)curr_nt->lastbreakpoint) ^ notation_item_is_preselected((t_notation_obj *) x, (t_notation_item *)curr_nt->lastbreakpoint)); 
							char note_unselected = false;
							char is_note_played, is_note_locked, is_note_muted, is_note_solo;
							double note_y_real, note_x_real;
							double ledger_lines_y[CONST_MAX_LEDGER_LINES]; 
							int num_ledger_lines; int i;
							long scaleposition;
							double notehead_uwidth;
							double end_pos = onset_to_xposition((t_notation_obj *) x,curr_ch->onset+curr_nt->duration, NULL);
							t_bpt *selected_breakpoint = NULL;
							
							if (is_chord_selected && is_note_preselected)
								note_unselected = true;
							
							is_note_locked = notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt);
							is_note_muted = notation_item_is_globally_muted((t_notation_obj *)x, (t_notation_item *)curr_nt); 
							is_note_solo = notation_item_is_globally_solo((t_notation_obj *)x, (t_notation_item *)curr_nt); 
							is_note_played = x->r_ob.highlight_played_notes ? (should_element_be_played((t_notation_obj *) x, (t_notation_item *)curr_nt) && (curr_ch->played || curr_nt->played)) : false; 
							
							// get colors
							notecolor = note_get_color((t_notation_obj *) x, curr_nt, !note_unselected && (is_chord_selected || is_note_selected || is_durationline_selected), is_note_played, is_note_locked, is_note_muted, is_note_solo, is_chord_linear_edited, curr_nt->velocity);
                            durationlinecolor = get_durationline_color((t_notation_obj *) x, curr_nt, !note_unselected && (is_chord_selected || is_note_selected || is_durationline_selected), is_note_played, is_note_locked, is_note_muted, is_note_solo, is_chord_linear_edited, curr_nt->velocity);
                            accidentalcolor = get_accidental_color((t_notation_obj *) x, curr_nt, !note_unselected && (is_chord_selected || is_note_selected || is_durationline_selected), is_note_played, is_note_locked, is_note_muted, is_note_solo, is_chord_linear_edited, curr_nt->velocity);
							tailcolor = get_tail_color((t_notation_obj *) x, curr_nt, !note_unselected && (is_chord_selected || is_note_selected || is_durationline_selected || is_tail_selected), is_note_played, is_note_locked, is_note_muted, is_note_solo, is_chord_linear_edited, x->r_ob.breakpoints_have_velocity ? curr_nt->lastbreakpoint->velocity : curr_nt->velocity);
							
							// finding y positions
							note_y_real = system_shift + mc_to_yposition_in_scale((t_notation_obj *) x, note_get_screen_midicents(curr_nt), (t_voice *) voice);
							curr_nt->center.y = note_y_real;
							
							// finding x positions
							notehead_uwidth = curr_nt->notehead_uwidth;
							note_x_real = stem_x + get_notehead_ux_shift((t_notation_obj *) x, curr_nt) * x->r_ob.zoom_y + curr_nt->notecenter_stem_delta_ux * x->r_ob.zoom_y;
							curr_nt->center.x = note_x_real; // center of the notehead
                            
							// paint note label circles
							if (x->r_ob.show_label_families == k_SHOW_LABEL_FAMILIES_SINGLETONS) 
								paint_note_label_families((t_notation_obj *)x, view, g, curr_nt, &label_family_chord_shape_radius);
							
							// background noteslots
							double slot_zoom_y = x->r_ob.zoom_y * (x->r_ob.bgslot_zoom/100.);
                            double note_x = note_x_real - ((curr_nt->notehead_uwidth / 2.) * x->r_ob.zoom_y);
							paint_background_slots((t_notation_obj *) x, g, note_x_real, note_y_real, end_pos - note_x_real, jf_text_small,
                                                    jf_text_smallbold, jf_small_dynamics, (t_notation_item *)curr_nt,
													note_x_real + (note_x_real - note_x) * 2 + x->r_ob.background_slot_text_ushift[0] * slot_zoom_y, 
													note_y_real + x->r_ob.background_slot_text_ushift[1] * slot_zoom_y, 
													note_x_real + (note_x_real - note_x) * 2 + x->r_ob.background_slot_text_ushift[0] * slot_zoom_y, 
													note_y_real - (1.3 * x->r_ob.slot_background_font_size - 0.5) * slot_zoom_y + x->r_ob.background_slot_text_ushift[1] * slot_zoom_y, -1);
							
							// draw ledger lines if needed
							scaleposition = curr_nt->scaleposition;
							get_ledger_lines((t_notation_obj *) x, (t_voice *) voice, scaleposition, &num_ledger_lines, ledger_lines_y); // let's obtain the list of ledger lines y
							for (i = 0; i < num_ledger_lines; i++)
								paint_line(g, x->r_ob.j_mainstaves_rgba, note_x_real - CONST_LEDGER_LINES_HALF_UWIDTH * curr_nt->notehead_resize * x->r_ob.zoom_y, system_shift + ledger_lines_y[i], 
										   note_x_real + CONST_LEDGER_LINES_HALF_UWIDTH * curr_nt->notehead_resize * x->r_ob.zoom_y, system_shift + ledger_lines_y[i], 1.);
							
                            
//                            paint_line(g, build_jrgba(1, 0, 0, 0.5), chord_alignment_x, 0, chord_alignment_x, rect.height, 1.);
//                            dev_post("note voice %ld; alignment_pt: %.2f, stem_x: %.2f, notehead_width: %.2f", voice->v_ob.number + 1,chord_alignment_x, stem_x, curr_nt->notehead_uwidth * x->r_ob.zoom_y);

							// duration line and breakpoints
							paint_duration_line((t_notation_obj *) x, view, g, durationlinecolor, tailcolor, curr_nt, end_pos, system_shift, system_jump, note_unselected, is_chord_selected, is_note_selected, is_durationline_selected, is_note_played, is_note_locked, is_note_muted, is_note_solo, &selected_breakpoint);

                            // draw the notehead
                            paint_notehead((t_notation_obj *) x, view, g, jf, &notecolor, curr_nt, note_x_real, note_y_real, system_shift, 1.);
                            
#ifdef BACH_PAINT_IDS
                            if (curr_nt->r_it.ID > 0) {
                                char text[140];
                                snprintf_zero(text, 140, "%ld", curr_nt->r_it.ID);
                                write_text(g, jf_text_markers, build_jrgba(0.3, 0.2, 0.5, 1), text, note_x + stem_adj_x + notehead_uwidth * x->r_ob.zoom_y,
                                           note_y_real, rect.width, 40, JGRAPHICS_TEXT_JUSTIFICATION_LEFT + JGRAPHICS_TEXT_JUSTIFICATION_TOP, true, false);
                            }
#endif
                            
                            // draw the auxiliary stems, if needed
                            if (x->r_ob.show_stems > 1 && curr_nt->need_auxiliary_stem) {
                                if (curr_ch->direction == 1) { // stem down
                                    paint_line(g, stemcolor, stem_x, last_note_y_real - 0.4*octave_stem_length, note_x_real, last_note_y_real - 0.2*octave_stem_length, CONST_AUX_STEM_WIDTH);
                                    paint_line(g, stemcolor, note_x_real, last_note_y_real - 0.2*octave_stem_length, note_x_real, note_y_real - 0.5 * x->r_ob.step_y, CONST_AUX_STEM_WIDTH);
                                } else if (curr_ch->direction == -1) { // stem up
                                    paint_line(g, stemcolor, stem_x, first_note_y_real + 0.4*octave_stem_length, note_x_real, first_note_y_real + 0.2*octave_stem_length, CONST_AUX_STEM_WIDTH);
                                    paint_line(g, stemcolor, note_x_real, first_note_y_real + 0.2*octave_stem_length, note_x_real, note_y_real + 0.5 * x->r_ob.step_y, CONST_AUX_STEM_WIDTH);
                                } else {
                                    object_warn((t_object *)x, "Warning: chord direction undefined!");
                                }
                            }
                            
                            // need to put accidentals?
                            paint_noteaccidentals((t_notation_obj *) x, g, jf_acc, jf_text_fractions, jf_acc_bogus, &accidentalcolor, curr_nt, get_voice_clef((t_notation_obj *)x, (t_voice *)voice), note_y_real, stem_x, NULL, NULL);
                            
                            
						}
					}
                    
                    // setting bottommost and topmost y values
                    curr_ch->bottommost_y_noacc = curr_ch->firstnote->center.y + (curr_ch->direction < 0 ? octave_stem_length : x->r_ob.step_y);
                    curr_ch->topmost_y_noacc = curr_ch->lastnote->center.y - (curr_ch->direction > 0 ? octave_stem_length : x->r_ob.step_y);
                    
                    // painting articulations
                    // need to put articulations?
                    if (x->r_ob.show_articulations){
                        if (x->r_ob.link_articulations_to_slot > 0 && x->r_ob.link_articulations_to_slot < CONST_MAX_SLOTS) {
                            long s = x->r_ob.link_articulations_to_slot - 1;
                            if (x->r_ob.slotinfo[s].slot_type == k_SLOT_TYPE_ARTICULATIONS) {
                                // "NEW WAY" (which, for bach.roll, is the ONLY way): slot-attached articulations
                                for (curr_nt = curr_ch->firstnote; curr_nt; curr_nt = curr_nt->next) {
                                    t_slotitem *item;
                                    for (item = curr_nt->slot[s].firstitem; item; item = item->next) {
                                        t_articulation *art = (t_articulation *)item->item;
                                        char is_note_locked = notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt);
                                        char is_note_muted = notation_item_is_globally_muted((t_notation_obj *)x, (t_notation_item *)curr_nt);
                                        char is_note_solo = notation_item_is_globally_solo((t_notation_obj *)x, (t_notation_item *)curr_nt);
                                        char is_note_played = x->r_ob.highlight_played_notes ? (should_element_be_played((t_notation_obj *) x, (t_notation_item *)curr_nt) && (curr_ch->played || curr_nt->played)) : false;
                                        char is_articulation_selected = notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)art);
                                        double end_pos = onset_to_xposition((t_notation_obj *) x,curr_ch->onset+curr_nt->duration, NULL);
                                        t_jrgba articulationcolor = get_articulation_color((t_notation_obj *) x, curr_ch, is_articulation_selected, is_note_played, is_note_locked, is_note_muted, is_note_solo, is_chord_linear_edited);
                                        paint_articulation((t_notation_obj *) x, g, &articulationcolor, art, (t_notation_item *)curr_nt, curr_ch->direction, stem_x, curr_nt->center.x, curr_nt->center.y, curr_nt->notehead_uwidth, end_pos, part_direction);
                                    }
                                }
                            }
                        }
                    }

                    if (x->r_ob.show_annotations){
                        if (x->r_ob.link_annotation_to_slot > 0 && x->r_ob.link_annotation_to_slot < CONST_MAX_SLOTS) {
                            long s = x->r_ob.link_annotation_to_slot - 1;
                            for (curr_nt = curr_ch->firstnote; curr_nt; curr_nt = curr_nt->next) {
                                if (notation_item_get_slot_firstitem((t_notation_obj *)x, (t_notation_item *)curr_nt, s)) {
                                    char is_note_locked = notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt);
                                    char is_note_muted = notation_item_is_globally_muted((t_notation_obj *)x, (t_notation_item *)curr_nt);
                                    char is_note_solo = notation_item_is_globally_solo((t_notation_obj *)x, (t_notation_item *)curr_nt);
                                    char is_note_played = x->r_ob.highlight_played_notes ? (should_element_be_played((t_notation_obj *) x, (t_notation_item *)curr_nt) && (curr_ch->played || curr_nt->played)) : false;
                                    t_jrgba annotationcolor = get_annotation_color((t_notation_obj *) x, curr_ch, false, is_note_played, is_note_locked, is_note_muted, is_note_solo, is_chord_linear_edited);
                                    paint_annotation_from_slot((t_notation_obj *) x, g, &annotationcolor, (t_notation_item *)curr_nt, curr_nt->notehead_textbox_left_corner.x, s, jf_ann, staff_top_y);
                                }
                            }
                        }
                    }

                    
                    /*
                    if (x->r_ob.show_dynamics || x->r_ob.show_hairpins){
                        if (x->r_ob.link_dynamics_to_slot > 0 && x->r_ob.link_dynamics_to_slot < CONST_MAX_SLOTS) {
                            if (!(x->r_ob.is_editing_type == k_DYNAMICS && x->r_ob.is_editing_chord == curr_ch)){
                                long s = x->r_ob.link_dynamics_to_slot - 1;
                                for (curr_nt = curr_ch->firstnote; curr_nt; curr_nt = curr_nt->next) {
                                    if (notation_item_get_slot_firstitem((t_notation_obj *)x, (t_notation_item *)curr_nt, s)) {
                                        char is_note_locked = notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt);
                                        char is_note_muted = notation_item_is_globally_muted((t_notation_obj *)x, (t_notation_item *)curr_nt);
                                        char is_note_solo = notation_item_is_globally_solo((t_notation_obj *)x, (t_notation_item *)curr_nt);
                                        char is_note_played = x->r_ob.highlight_played_notes ? (should_element_be_played((t_notation_obj *) x, (t_notation_item *)curr_nt) && (curr_ch->played || curr_nt->played)) : false;
                                        t_jrgba dynamicscolor = get_dynamics_color((t_notation_obj *) x, curr_ch, false, is_note_played, is_note_locked, is_note_muted, is_note_solo, is_chord_linear_edited);
                                        double end_pos = onset_to_xposition((t_notation_obj *) x, curr_ch->onset + chord_get_max_duration((t_notation_obj *)x, curr_ch), NULL);
                                        paint_dynamics_from_slot((t_notation_obj *)x, g, &dynamicscolor, (t_notation_item *)curr_nt, chord_alignment_x, end_pos - chord_alignment_x, s, jf_dynamics, staff_bottom_y - x->r_ob.dynamics_uy_pos * x->r_ob.zoom_y, &curr_hairpin_start_x, &curr_hairpin_type, false);
                                        break; // WE ONLY ACCOUNT FOR THE FIRST NOTE BEARING A DYNAMICS!!!
                                    }
                                }
                            }
                        }
                    } */
                    
                    if (x->r_ob.show_dynamics || x->r_ob.show_hairpins){
                        if (curr_ch->dynamics && curr_ch->dynamics->text) {
                            t_notation_item *nitem = (t_notation_item *)curr_ch;
                            char is_item_locked = notation_item_is_globally_locked((t_notation_obj *)x, nitem);
                            char is_item_muted = notation_item_is_globally_muted((t_notation_obj *)x, nitem);
                            char is_item_solo = notation_item_is_globally_solo((t_notation_obj *)x, nitem);
                            char is_item_played = x->r_ob.highlight_played_notes ? (should_element_be_played((t_notation_obj *) x, nitem) && curr_ch->played) : false;
                            char is_dynamics_selected = notation_item_is_selected((t_notation_obj *) x, nitem) || notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)curr_ch->dynamics);
                            
                            t_jrgba dynamicscolor = get_dynamics_color((t_notation_obj *) x, curr_ch, is_dynamics_selected, is_item_played, is_item_locked, is_item_muted, is_item_solo, is_chord_linear_edited);
                            double chord_alignment_x = chord_get_alignment_x((t_notation_obj *)x, curr_ch);
                            
                            double end_pos = onset_to_xposition((t_notation_obj *) x, curr_ch->onset + chord_get_max_duration((t_notation_obj *)x, curr_ch), NULL);
                            
                            paint_dynamics_from_symbol((t_notation_obj *)x, g, &dynamicscolor, nitem, chord_alignment_x, end_pos - chord_alignment_x, curr_ch->dynamics->text, jf_dynamics, x->r_ob.dynamics_font_size * x->r_ob.zoom_y, staff_bottom_y - x->r_ob.dynamics_uy_pos * x->r_ob.zoom_y, &curr_hairpin_start_x, &curr_hairpin_type, &prev_hairpin_color, &prev_hairpin_dontpaint, false);
                        }
                    }

                    
					// paint chord label rectangles
					if (x->r_ob.show_label_families == k_SHOW_LABEL_FAMILIES_SINGLETONS)
						paint_chord_label_families((t_notation_obj *)x, view, g, curr_ch, label_family_chord_shape_radius);
					
					// group lines?
					if ((x->r_ob.show_groups % 2) && curr_ch->r_it.group){
						t_notation_item *tmp;
						if (x->r_ob.show_stems == 0 && curr_ch->firstnote != curr_ch->lastnote) {
							t_note *nt;
							for (nt = curr_ch->firstnote; nt && nt->next; nt = nt->next)
								paint_dashed_line(g, stemcolor, stem_x + nt->notecenter_stem_delta_ux * x->r_ob.zoom_y, staff_top_y + nt->center_stafftop_uy * x->r_ob.zoom_y, 
												  stem_x + nt->next->notecenter_stem_delta_ux * x->r_ob.zoom_y, staff_top_y + nt->next->center_stafftop_uy * x->r_ob.zoom_y, 0.5, 2);
						}
						for (tmp = curr_ch->r_it.group->firstelem; tmp; tmp = tmp->next_group_item){
							if (tmp->type == k_CHORD && (t_chord *)tmp != curr_ch){
								t_chord *tmpch = (t_chord *) tmp;
								if (tmpch->onset <= curr_ch->onset) {
									double tmp_staff_top_y = (tmpch->voiceparent == curr_ch->voiceparent ? staff_top_y : get_staff_top_y((t_notation_obj *) x, (t_voice *)tmpch->voiceparent, false));
									paint_dashed_line(g, stemcolor, onset_to_xposition((t_notation_obj *) x, tmpch->onset, NULL), tmp_staff_top_y + (x->r_ob.show_stems > 0 ? tmpch->stemtip_stafftop_uy : (tmpch->direction == -1 ? tmpch->bottommostnote_stafftop_uy : tmpch->topmostnote_stafftop_uy)) * x->r_ob.zoom_y,
													  stem_x, staff_top_y + (x->r_ob.show_stems > 0 ? curr_ch->stemtip_stafftop_uy : (curr_ch->direction == -1 ? curr_ch->bottommostnote_stafftop_uy : curr_ch->topmostnote_stafftop_uy)) * x->r_ob.zoom_y, 0.5, 2);
								}
							}
						}
					}
					
					// lyrics?
					if (curr_ch->lyrics && curr_ch->lyrics->label && x->r_ob.show_lyrics) {
						double pos_y = staff_bottom_y - x->r_ob.lyrics_uy_pos * x->r_ob.zoom_y;
						double pos_x = curr_ch->stem_x + ((curr_ch->direction == 1 ? -0.5 : 0.5) * get_principal_notehead_uwidth((t_notation_obj *)x, curr_ch) + curr_ch->lyrics->lyrics_ux_shift) * x->r_ob.zoom_y;
						if (!(x->r_ob.is_editing_type == k_LYRICS && x->r_ob.is_editing_chord == curr_ch)){
							char is_lyrics_selected = notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)curr_ch) || notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)curr_ch->lyrics);
							t_jrgba lyrics_color = change_color_depending_on_playlockmute((t_notation_obj *) x, x->r_ob.j_lyrics_rgba, is_lyrics_selected, is_chord_played, is_chord_locked, is_chord_muted, is_chord_solo, false);
							write_text_account_for_vinset((t_notation_obj *) x, g, jf_lyrics, lyrics_color, curr_ch->lyrics->label, pos_x, pos_y);
						}
						
						if (lyrics_dashed_going_on) {
							double this_left_x = pos_x;
							long num_dash_needed = MAX(0, round((this_left_x - left_dashed_x) * CONST_NUM_DASH_PER_UX)); 
							
							if (num_dash_needed == 0 && (this_left_x - left_dashed_x) > CONST_UX_MINIMUM_SPACE_FOR_DASH * 1.25)
								num_dash_needed = 1;
							
							if (num_dash_needed == 1) {
								double x_module = (this_left_x - left_dashed_x - CONST_UX_MINIMUM_SPACE_FOR_DASH)/2;
								write_text_account_for_vinset((t_notation_obj *) x, g, jf_lyrics, x->r_ob.j_lyrics_rgba, "-", left_dashed_x + x_module, pos_y);
							} else if (num_dash_needed > 1) {
								long i;
								double x_module = (this_left_x - left_dashed_x)/num_dash_needed;
								for (i = 0; i < num_dash_needed; i++){
									double this_x = left_dashed_x + x_module/2. + i * x_module;
									write_text_account_for_vinset((t_notation_obj *) x, g, jf_lyrics, x->r_ob.j_lyrics_rgba, "-", this_x, pos_y);
								}
							}
						}
						
						lyrics_dashed_going_on = curr_ch->lyrics->lyrics_dashed_extension;
						if (lyrics_dashed_going_on)
							left_dashed_x = pos_x + curr_ch->lyrics->lyrics_uwidth * x->r_ob.zoom_y;
						
					}
					
					
				} else if (position > 0)
                    break;
			}
            
            
            // last hairpin?
            if (curr_hairpin_type && x->r_ob.show_hairpins){
                long s = x->r_ob.link_dynamics_to_slot - 1;
                if (s >= 0 && s < CONST_MAX_SLOTS && x->r_ob.slotinfo[s].slot_type == k_SLOT_TYPE_DYNAMICS) {
                    long old_hairpin_type = curr_hairpin_type;
                    t_chord *lastch = get_next_chord_containing_dynamics((t_notation_obj *)x, curr_ch, &curr_hairpin_type, false, true);
                    double curr_hairpin_end_x = rect.width * 2;
                    if (lastch)
                        curr_hairpin_end_x = onset_to_xposition((t_notation_obj *)x, lastch->onset, NULL);
                    paint_dynamics_from_symbol((t_notation_obj *)x, g, NULL, NULL, curr_hairpin_end_x, 0, NULL, jf_dynamics, x->r_ob.dynamics_font_size * x->r_ob.zoom_y, staff_bottom_y - x->r_ob.dynamics_uy_pos * x->r_ob.zoom_y, &curr_hairpin_start_x, &old_hairpin_type, &prev_hairpin_color, &prev_hairpin_dontpaint, false);
                }
            }

			
			// dashed lines for lyrics left to draw?
			if (x->r_ob.show_lyrics && lyrics_dashed_going_on) {
				double pos_y = staff_bottom_y - x->r_ob.lyrics_uy_pos * x->r_ob.zoom_y;
				double this_left_x = x->r_ob.j_inset_x + x->r_ob.inner_width;
				long num_dash_needed = MAX(0, round((this_left_x - left_dashed_x) * CONST_NUM_DASH_PER_UX)); 
				
				if (num_dash_needed == 1) {
					double x_module = (this_left_x - left_dashed_x)/2;
					write_text_account_for_vinset((t_notation_obj *) x, g, jf_lyrics, x->r_ob.j_lyrics_rgba, "-", left_dashed_x + x_module, pos_y);
				} else if (num_dash_needed > 1) {
					long i;
					double x_module = (this_left_x - left_dashed_x)/num_dash_needed;
					for (i = 0; i < num_dash_needed; i++){
						double this_x = left_dashed_x + x_module/2. + i * x_module;
						write_text_account_for_vinset((t_notation_obj *) x, g, jf_lyrics, x->r_ob.j_lyrics_rgba, "-", this_x, pos_y);
					}
				}
			}
            
            
            // Linear editing the voice?
            // linear_edit cursor?
            if (voice_linear_edited){
                double yy, ypos = voice->v_ob.middleC_y - x->r_ob.notation_cursor.step * x->r_ob.step_y;
                double xpos = onset_to_xposition((t_notation_obj *) x, x->r_ob.notation_cursor.onset, NULL);
                paint_line(g, x->r_ob.j_linear_edit_rgba, xpos, staff_top_y, xpos, staff_bottom_y, 2);

                // ledger lines?
                if (ypos < staff_top_y) {
                    for (yy = staff_top_y - x->r_ob.step_y * 2; yy > ypos; yy -= x->r_ob.step_y * 2)
                        paint_line(g, x->r_ob.j_auxiliarystaves_rgba, xpos - 1.3 * x->r_ob.zoom_y, yy, xpos + 1.3 * x->r_ob.zoom_y, yy, 1);
                    paint_line(g, x->r_ob.j_linear_edit_rgba, xpos, staff_top_y, xpos, ypos, 1);
                }
                if (ypos > staff_bottom_y) {
                    for (yy = staff_bottom_y + x->r_ob.step_y * 2; yy < ypos; yy += x->r_ob.step_y * 2)
                        paint_line(g, x->r_ob.j_mainstaves_rgba, xpos - 1.3 * x->r_ob.zoom_y, yy, xpos + 1.3 * x->r_ob.zoom_y, yy, 1);
                    paint_line(g, x->r_ob.j_linear_edit_rgba, xpos, staff_bottom_y, xpos, ypos, 1);
                }
                
                // paint tick
                paint_line(g, x->r_ob.j_linear_edit_rgba, xpos - 2 * x->r_ob.zoom_y, ypos, xpos + 2 * x->r_ob.zoom_y, ypos, 2);
            }
			
			// TODO: repaint selection!
			// the selction must be in the foreground
		}
        
		unlock_general_mutex((t_notation_obj *)x);
		
		// markers, if any
		if (x->r_ob.show_markers && x->r_ob.firstmarker) {
			t_marker *marker;
            
            lock_markers_mutex((t_notation_obj *)x);
            markers_check_update_name_uwidth((t_notation_obj *)x);
			for (marker = x->r_ob.firstmarker; marker; marker = marker->next) {
                if (marker->need_update_name_uwidth)
                    recalculate_marker_name_uwidth((t_notation_obj *)x, marker);
				double marker_onset = marker->position_ms;
				char buf[1000];
				char is_marker_selected = notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)marker);
				char is_marker_preselected = notation_item_is_preselected((t_notation_obj *) x, (t_notation_item *)marker);
				get_names_as_text(marker->r_it.names, buf, 1000);
				if (marker_onset >= x->r_ob.screen_ms_start - 200 / x->r_ob.zoom_x && marker_onset < x->r_ob.screen_ms_end) {
					marker->name_painted_direction = (marker_onset + deltaxpixels_to_deltaonset((t_notation_obj *)x, marker->name_uwidth) > x->r_ob.screen_ms_end ? -1 : 1);
					paint_marker((t_notation_obj *) x, g, (is_marker_selected ^ is_marker_preselected) ? x->r_ob.j_selection_rgba : x->r_ob.j_marker_rgba, 
								 jf_text_markers, marker, onset_to_xposition((t_notation_obj *)x, marker_onset, NULL), 
								 playhead_y1, playhead_y2, 2., x->r_ob.is_editing_type != k_MARKERNAME || x->r_ob.is_editing_marker != marker);
				} else if (marker_onset >= x->r_ob.screen_ms_end)
                    break;
			}
			unlock_markers_mutex((t_notation_obj *)x);;
		}
		
		// destroying all fonts
		jfont_destroy_debug(jf_text_small);
		jfont_destroy_debug(jf_text_smallbold);
		jfont_destroy_debug(jf_text_markers);
		jfont_destroy_debug(jf_lyrics_nozoom);
		jfont_destroy_debug(jf_lyrics);
        jfont_destroy_debug(jf_ann);
        jfont_destroy_debug(jf_dynamics);
        jfont_destroy_debug(jf_dynamics_nozoom);
        jfont_destroy_debug(jf_small_dynamics);
		jbox_end_layer((t_object *)x, view, gensym("static_layer1"));
	}
	
	jbox_paint_layer((t_object *)x, view, gensym("static_layer1"), 0., 0.);	// position of the layer
}


void paint_static_stuff2(t_roll *x, t_object *view, t_rect rect, t_jfont *jf, t_jfont *jf_acc, t_jfont *jf_acc_bogus, t_jfont *jf_text_legend)
{
	
	t_jgraphics *g = jbox_start_layer((t_object *)x, view, gensym("static_layer2"), rect.width, rect.height);
	
	if (g) {
		t_jfont *jf_voice_names = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, x->r_ob.voice_names_font_size * x->r_ob.zoom_y); 
		double system_jump = x->r_ob.system_jump;
        
        t_rollvoice *voice;
/*        double nu_end_x_to_repaint_no_inset = unscaled_xposition_to_xposition((t_notation_obj *) x, x->r_ob.screen_ux_start) - CONST_X_LEFT_START_DELETE_UX_ROLL - x->r_ob.additional_ux_start_pad * x->r_ob.zoom_y;
        double nu_fadestart_no_inset = unscaled_xposition_to_xposition((t_notation_obj *) x, x->r_ob.screen_ux_start) - CONST_X_LEFT_START_FADE_UX_ROLL - x->r_ob.additional_ux_start_pad * x->r_ob.zoom_y;
  */
        double end_x_to_repaint_no_inset = (22 + x->r_ob.key_signature_uwidth + x->r_ob.voice_names_uwidth + x->r_ob.additional_ux_start_pad) * x->r_ob.zoom_y - x->r_ob.additional_ux_start_pad * x->r_ob.zoom_y + x->r_ob.j_inset_x;
        double fadestart_no_inset = (15 + x->r_ob.key_signature_uwidth + x->r_ob.voice_names_uwidth + x->r_ob.additional_ux_start_pad) * x->r_ob.zoom_y - x->r_ob.additional_ux_start_pad * x->r_ob.zoom_y + x->r_ob.j_inset_x;

/*        double old_fadestart_no_inset = onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_start - CONST_X_LEFT_START_FADE_MS / x->r_ob.zoom_x, NULL) - x->r_ob.additional_ux_start_pad * x->r_ob.zoom_y;
        double old_end_x_to_repaint_no_inset = onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_start - CONST_X_LEFT_START_DELETE_MS / x->r_ob.zoom_x, NULL) - x->r_ob.additional_ux_start_pad * x->r_ob.zoom_y;

        paint_line(g, build_jrgba(1, 0, 0, 1), end_x_to_repaint_no_inset, 0, end_x_to_repaint_no_inset, 1000, 1);
        paint_line(g, build_jrgba(0, 1, 0, 1), fadestart_no_inset, 0, fadestart_no_inset, 1000, 1);
        paint_dashed_line(g, build_jrgba(1, 0, 0, 1), old_end_x_to_repaint_no_inset, 0, old_end_x_to_repaint_no_inset, 1000, 1, 5);
        paint_dashed_line(g, build_jrgba(0, 1, 0, 1), old_fadestart_no_inset, 0, old_fadestart_no_inset, 1000, 1, 5);
 */
        lock_general_mutex((t_notation_obj *)x);
		
        // painting label families
        if (x->r_ob.show_label_families == k_SHOW_LABEL_FAMILIES_BOUNDINGBOX || x->r_ob.show_label_families == k_SHOW_LABEL_FAMILIES_VENN)
            paint_venn_label_families((t_notation_obj *)x, view, g);

		// painting ruler&grids
		paint_ruler_and_grid_for_roll((t_notation_obj *)x, g, rect);
		
		// repainting left part
		repaint_left_background_part((t_notation_obj *)x, g, rect, fadestart_no_inset, end_x_to_repaint_no_inset);
		
		for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
			double k; 
			long clef = get_voice_clef((t_notation_obj *)x, (t_voice *)voice);
			t_jrgba mainstaffcolor = get_mainstaff_color((t_notation_obj *) x, voice->v_ob.r_it.selected, voice->v_ob.locked, voice->v_ob.muted, voice->v_ob.solo);
            t_jrgba keysigcolor = get_keysig_color((t_notation_obj *) x, voice->v_ob.r_it.selected, voice->v_ob.locked, voice->v_ob.muted, voice->v_ob.solo);
			t_jrgba auxstaffcolor = get_auxstaff_color((t_notation_obj *) x, voice->v_ob.r_it.selected, voice->v_ob.locked, voice->v_ob.muted, voice->v_ob.solo);
			t_jrgba clefcolor = get_clef_color((t_notation_obj *) x, voice->v_ob.r_it.selected, voice->v_ob.locked, voice->v_ob.muted, voice->v_ob.solo);
            double staff_top_y = get_staff_top_y((t_notation_obj *) x, (t_voice *) voice, true);
            double staff_bottom_y = get_staff_bottom_y((t_notation_obj *) x, (t_voice *) voice, true);

            if (voice->v_ob.hidden)
				continue;
			
            if (voice->v_ob.part_index != 0)
                continue;
            
			// repaint first parts of staves
            for (k=x->r_ob.first_shown_system; k <= x->r_ob.last_shown_system; k++)
                    paint_staff_lines((t_notation_obj *)x, g, x->r_ob.j_inset_x + x->r_ob.voice_names_uwidth * x->r_ob.zoom_y, end_x_to_repaint_no_inset, 1., voice->v_ob.middleC_y + k * system_jump, clef, mainstaffcolor, auxstaffcolor, voice->v_ob.num_staff_lines, voice->v_ob.staff_lines);
			
			// paint clefs
			for (k=x->r_ob.first_shown_system; k <= x->r_ob.last_shown_system; k++)
				paint_clef((t_notation_obj *)x, g, jf, voice->v_ob.middleC_y + k * system_jump, clef, clefcolor);
			
			// paint key signature
			for (k=x->r_ob.first_shown_system; k <= x->r_ob.last_shown_system; k++)
				paint_keysignature((t_notation_obj *)x, g, jf_acc, jf_acc_bogus, voice->v_ob.middleC_y + k * system_jump, (t_voice *)voice, keysigcolor);
			
            // paint the vertical staff line
            if (is_clef_multistaff((t_notation_obj *)x, clef))
                paint_left_vertical_staffline((t_notation_obj *)x, g, (t_voice *)voice, mainstaffcolor);

            // paint the accollatura
            if (voiceensemble_get_numparts((t_notation_obj *)x, (t_voice *)voice) > 1)
                for (k=x->r_ob.first_shown_system; k <= x->r_ob.last_shown_system; k++)
                    paint_accollatura((t_notation_obj *)x, g, staff_top_y, staff_bottom_y, mainstaffcolor);
			
			// paint voice names
			if (x->r_ob.there_are_voice_names && x->r_ob.show_voice_names && (x->r_ob.is_editing_type != k_VOICENAME || x->r_ob.is_editing_voice_name != voice->v_ob.number)) { 
				char buf[1000];
                get_voicenames_as_text((t_notation_obj *)x, (t_voice *)voice, buf, 1000);
				for (k=x->r_ob.first_shown_system; k <= x->r_ob.last_shown_system; k++)
					write_voicename((t_notation_obj *)x, g, jf_voice_names, (staff_top_y + staff_bottom_y)/2, buf, x->r_ob.voice_names_alignment, mainstaffcolor);
			}
                
		}
		
		unlock_general_mutex((t_notation_obj *)x);

		// strip of background at the end
		paint_filledrectangle(g, x->r_ob.j_background_rgba, x->r_ob.j_inset_x + x->r_ob.inner_width, 0, x->r_ob.j_inset_x, rect.height);
		
        // obtain and paint legend
        notationobj_paint_legend((t_notation_obj *)x, g, rect, jf_text_legend);
		
		// draw vertical scroll bar
		update_vscrollbar_and_paint_it_if_needed((t_notation_obj *) x, g, rect);
		
		// draw horizontal scroll bar
		if (x->r_ob.view == k_VIEW_SCROLL)
			update_hscrollbar_and_paint_it_if_needed((t_notation_obj *) x, g, rect);
		
		// slots
		if (x->r_ob.active_slot_num > -1 && x->r_ob.active_slot_notationitem) {
			long system1 = -1; // negative: will be filled
			
			// determine the slot window x
			lock_general_mutex((t_notation_obj *)x);
			if (x->r_ob.slotinfo[x->r_ob.active_slot_num].slot_uwidth < 0) { //temporal slot
				x->r_ob.slot_window_ms1 = notation_item_get_onset_ms((t_notation_obj *)x, x->r_ob.active_slot_notationitem);
				x->r_ob.slot_window_ms2 = x->r_ob.slot_window_ms1 + notation_item_get_duration_ms((t_notation_obj *)x, x->r_ob.active_slot_notationitem);
				
				x->r_ob.slot_window_x1 = onset_to_xposition((t_notation_obj *) x, x->r_ob.slot_window_ms1, &system1);
				x->r_ob.slot_window_x2 = onset_to_xposition((t_notation_obj *) x, x->r_ob.slot_window_ms2, &system1); // we want it on the same system
			} else {
                x->r_ob.slot_window_ms1 = notation_item_get_onset_ms((t_notation_obj *)x, x->r_ob.active_slot_notationitem);
				x->r_ob.slot_window_x1 = onset_to_xposition((t_notation_obj *) x, x->r_ob.slot_window_ms1, &system1);
				x->r_ob.slot_window_x2 = x->r_ob.slot_window_x1 + x->r_ob.slotinfo[x->r_ob.active_slot_num].slot_uwidth * x->r_ob.zoom_y * (x->r_ob.slot_window_zoom / 100.);
				x->r_ob.slot_window_ms2 = xposition_to_onset((t_notation_obj *) x, x->r_ob.slot_window_x2, system1);
			}
			paint_slot((t_notation_obj *) x, g, rect, x->r_ob.active_slot_notationitem, x->r_ob.active_slot_num);
			unlock_general_mutex((t_notation_obj *)x);
		}
		
		
		jfont_destroy_debug(jf_voice_names);
		jbox_end_layer((t_object *)x, view, gensym("static_layer2"));
	}
	
	jbox_paint_layer((t_object *)x, view, gensym("static_layer2"), 0., 0.);	// position of the layer
}



void roll_paint(t_roll *x, t_object *view)
{
	t_jgraphics *g;
	t_rect rect;
//	char firsttime = x->r_ob.firsttime;
	char must_repaint = false;
	
	t_jfont *jf_text, *jf_text_fixed, *jf, *jf_acc, *jf_acc_bogus, *jf_text_fractions;
		
	char legend_text[256];
	
	if (x->r_ob.j_box.l_rebuild){
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
		x->r_ob.j_box.l_rebuild = 0;
	}
	
	// getting rectangle dimensions
	g = (t_jgraphics*) patcherview_get_jgraphics(view); 
	jbox_get_rect_for_view(&x->r_ob.j_box.l_box.b_ob, view, &rect);
	
	paint_background((t_object *)x, g, &rect, &x->r_ob.j_background_rgba, x->r_ob.corner_roundness);

	x->r_ob.inner_width = rect.width - (2 * x->r_ob.j_inset_x);
	x->r_ob.inner_height = rect.height - (2 * x->r_ob.j_inset_y); 
	x->r_ob.height = rect.height;
	x->r_ob.width = rect.width;

	// getting/keeping domain
	x->r_ob.domain = getdomain((t_notation_obj *) x);

    
	if (x->r_ob.zoom_y == 0) 
		x->r_ob.firsttime = 1;
	
	if (false && x->r_ob.firsttime) { // actually, it shouldn't be used anymore
		x->r_ob.system_jump = get_system_jump((t_notation_obj *)x);
		calculate_voice_offsets((t_notation_obj *) x);

		if (x->r_ob.link_vzoom_to_height)
			notationobj_set_vzoom_depending_on_height((t_notation_obj *) x, rect.height);
		else
			notationobj_reset_size_related_stuff((t_notation_obj *) x);

		x->r_ob.needed_uheight = notationobj_get_supposed_standard_height((t_notation_obj *) x);
		x->r_ob.needed_uheight_for_one_system = x->r_ob.needed_uheight / ((x->r_ob.num_systems > 0) ? x->r_ob.num_systems : 1);
		calculate_ms_on_a_line((t_notation_obj *) x);
		update_vscrollbar((t_notation_obj *) x, 0);
		redraw_hscrollbar((t_notation_obj *)x, 0);
		
		x->r_ob.firsttime = 0;

		// we need to recalculate 2 times the chord parameters the firsttime
		// this is due to the fact thats 1) we want some parameters to be computed IMMEDIATELY (not in paint function) when a roll is set
		// or recalled, but 2) some of the parameters depend on the vzoom, which in turn might depend on rect.height, which is given ONLY in the
		// current view, in the paint function. So we need to recalculate parameters twice, at the beginning
		recalculate_all_chord_parameters(x);
		must_repaint = true;
	}

	jf_text = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, round(11. * x->r_ob.zoom_y)); 
	jf_text_fixed = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, x->r_ob.legend_font_size); 
	jf = jfont_create_debug(x->r_ob.noteheads_font->s_name, JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, x->r_ob.notation_typo_preferences.base_pt * x->r_ob.zoom_y);
	jf_acc = jfont_create_debug(x->r_ob.accidentals_font->s_name, JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, x->r_ob.accidentals_typo_preferences.base_pt * x->r_ob.zoom_y);
	jf_acc_bogus = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_NORMAL, round(8.8 * x->r_ob.zoom_y)); 
	jf_text_fractions = jfont_create_debug("Arial", JGRAPHICS_FONT_SLANT_NORMAL, JGRAPHICS_FONT_WEIGHT_BOLD, CONST_TEXT_FRACTIONS_PT * x->r_ob.zoom_y);
		
	x->r_ob.step_y = CONST_STEP_UY * x->r_ob.zoom_y;
	

	if (USE_BITMAPS_FOR_STANDARD_QUARTERNOTEHEADS) // building surfaces for most common graphic notation elements
		notationobj_build_notation_item_surfaces((t_notation_obj *)x, view, rect);
	
	
	// setting alpha to 1 before painting layers! otherwise we have blending issues
	jgraphics_set_source_rgba(g, 0, 0, 0, 1);
	paint_static_stuff1(x, view, rect, jf, jf_acc, jf_text_fractions, jf_acc_bogus);


	// do we have to print the play_head line?
	if (x->r_ob.playing) { 
		double playhead_y1, playhead_y2;
		double play_head_pos = onset_to_xposition((t_notation_obj *) x, x->r_ob.play_head_ms, NULL);
		get_playhead_ypos((t_notation_obj *)x, rect, &playhead_y1, &playhead_y2);
		paint_playhead(g, x->r_ob.j_play_rgba, play_head_pos, playhead_y1, playhead_y2, 1., 3 * x->r_ob.zoom_y);
	} else if (x->r_ob.show_playhead) {
		double playhead_y1, playhead_y2;
		double play_head_pos = onset_to_xposition((t_notation_obj *) x, x->r_ob.play_head_start_ms, NULL);
		get_playhead_ypos((t_notation_obj *)x, rect, &playhead_y1, &playhead_y2);
		paint_playhead(g, x->r_ob.j_play_rgba, play_head_pos, playhead_y1, playhead_y2, 1., 3 * x->r_ob.zoom_y);
	}
	
	// setting alpha to 1 before painting layers! otherwise we have blending issues
	jgraphics_set_source_rgba(g, 0, 0, 0, 1);
	paint_static_stuff2(x, view, rect, jf, jf_acc, jf_acc_bogus, jf_text_fixed);
	
	// paint the selection rectangle, if needed
	if (x->r_ob.j_mousedown_obj_type == k_REGION)
		paint_selection_rectangle((t_notation_obj *) x, g, get_default_selection_rectangle_border_color(), get_default_selection_rectangle_fill_color());
	else if (x->r_ob.j_mousedown_obj_type == k_ZOOMING_REGION)
		paint_selection_rectangle((t_notation_obj *) x, g, get_default_selection_rectangle_border_color(), get_default_selection_rectangle_zooming_fill_color());
	else if (x->r_ob.show_dilation_rectangle)
		paint_dilation_rectangle((t_notation_obj *) x, g);
						
	// paint loop region?
	if (x->r_ob.show_loop_region) {
		double playhead_y1, playhead_y2;
		double x1 = onset_to_xposition((t_notation_obj *)x, x->r_ob.loop_region.start.position_ms, NULL);
		double x2 = onset_to_xposition((t_notation_obj *)x, x->r_ob.loop_region.end.position_ms, NULL);
		get_playhead_ypos((t_notation_obj *)x, rect, &playhead_y1, &playhead_y2);
		paint_loop_region((t_notation_obj *) x, g, rect, x->r_ob.j_loop_rgba, x1, x2, playhead_y1, playhead_y2, 1.);
	}
    
    // linear-edit or mouseover legend
    if (is_in_linear_edit_mode((t_notation_obj *)x)){
        char legend_text[1024], onset_text[256], step_text[256];
        time_to_char_buf((t_notation_obj *)x, x->r_ob.linear_edit_time_step, step_text, 256);
        time_to_char_buf((t_notation_obj *)x,  x->r_ob.notation_cursor.onset, onset_text, 256);
        snprintf_zero(legend_text, 1024, "Voice %ld   Onset %s   Cents %ld   Step %s", x->r_ob.notation_cursor.voice->number + 1, onset_text, x->r_ob.notation_cursor.midicents, step_text);
        
        write_text(g, jf_text_fixed, x->r_ob.j_linear_edit_rgba, legend_text, x->r_ob.j_inset_x, x->r_ob.j_inset_y,
                   x->r_ob.inner_width - (x->r_ob.need_vscrollbar && x->r_ob.show_vscrollbar ? CONST_YSCROLLBAR_UWIDTH * x->r_ob.zoom_y : 0) - 5,
                   x->r_ob.inner_height - (x->r_ob.need_hscrollbar && x->r_ob.show_hscrollbar ? CONST_XSCROLLBAR_UHEIGHT * x->r_ob.zoom_y : 0) - 5 * x->r_ob.zoom_y,
                   JGRAPHICS_TEXT_JUSTIFICATION_RIGHT + JGRAPHICS_TEXT_JUSTIFICATION_BOTTOM, true, false);
    } else if (x->r_ob.j_mouse_is_over && x->r_ob.legend == 2 && x->r_ob.j_mouse_x >= x->r_ob.j_inset_x && x->r_ob.j_mouse_x <= x->r_ob.j_inset_x + x->r_ob.inner_width){
		char legend_text[256], onset_text[256];
        time_to_char_buf((t_notation_obj *)x, xposition_to_onset((t_notation_obj *) x,x->r_ob.j_mouse_x, yposition_to_systemnumber((t_notation_obj *) x, x->r_ob.j_mouse_y)), onset_text, 256);
		if (x->r_ob.active_slot_num == -1)
			snprintf_zero(legend_text, 256, "Onset %s   Cents %.1f", onset_text, yposition_to_mc((t_notation_obj *)x,x->r_ob.j_mouse_y, NULL, NULL));
		else
			snprintf_zero(legend_text, 256, "Onset %s", onset_text);
		
		write_text(g, jf_text_fixed, x->r_ob.j_legend_rgba, legend_text, x->r_ob.j_inset_x, x->r_ob.j_inset_y, 
				   x->r_ob.inner_width - (x->r_ob.need_vscrollbar && x->r_ob.show_vscrollbar ? CONST_YSCROLLBAR_UWIDTH * x->r_ob.zoom_y : 0) - 5, 
				   x->r_ob.inner_height - (x->r_ob.need_hscrollbar && x->r_ob.show_hscrollbar ? CONST_XSCROLLBAR_UHEIGHT * x->r_ob.zoom_y : 0) - 5 * x->r_ob.zoom_y,
				   JGRAPHICS_TEXT_JUSTIFICATION_RIGHT + JGRAPHICS_TEXT_JUSTIFICATION_BOTTOM, true, false);
	}
	
	jfont_destroy_debug(jf);
	jfont_destroy_debug(jf_text_fixed);
	jfont_destroy_debug(jf_acc);
	jfont_destroy_debug(jf_acc_bogus);
	jfont_destroy_debug(jf_text_fractions);
	jfont_destroy_debug(jf_text);

	// sending rebuild done, if needed (will be deferred_low!!!)
	if (x->r_ob.need_send_rebuild_done_after_paint) {
		send_rebuild_done((t_notation_obj *) x);
		x->r_ob.need_send_rebuild_done_after_paint = false;
	}

    if (x->r_ob.are_there_solos)
        paint_border((t_object *)x, g, &rect, &x->r_ob.j_solo_rgba, 2.5, x->r_ob.corner_roundness);
    else
        paint_border((t_object *)x, g, &rect, &x->r_ob.j_border_rgba, (!x->r_ob.show_border) ? 0 : ((x->r_ob.j_has_focus && x->r_ob.show_focus) ? 2.5 : 1), x->r_ob.corner_roundness);

	send_changed_bang_and_automessage_if_needed((t_notation_obj *)x);
	
	if (must_repaint)
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
}


// INLET INTERACTION:
// - a llll in the first inlet reconstruct the whole roll
// - a llll in the following inlets sets a cache for onsets/cents/durations/velocities/extra
// - a bang in the first inlets change/builds things starting from the cache values


void roll_jsave(t_roll *x, t_dictionary *d)
{
	if (x->r_ob.save_data_with_patcher){
		t_llll *whole_info;
		if (x->r_ob.j_box.l_dictll) {
			llll_store_in_dictionary(x->r_ob.j_box.l_dictll, d, "whole_roll_data", NULL);
		} else {
            whole_info = get_roll_values_as_llll(x, x->r_ob.bwcompatible <= 7900 ? k_CONSIDER_FOR_SAVING_WITH_BW_COMPATIBILITY : k_CONSIDER_FOR_SAVING,
														 (e_header_elems) (k_HEADER_BODY + k_HEADER_SLOTINFO + k_HEADER_MARKERS + k_HEADER_GROUPS + k_HEADER_MIDICHANNELS + k_HEADER_COMMANDS + k_HEADER_ARTICULATIONINFO + k_HEADER_NOTEHEADINFO),
														 true, false); // clefs, keys, voicenames and parts are already saved as object attributes!//
			llll_store_in_dictionary(whole_info, d, "whole_roll_data", NULL);
			llll_free(whole_info);
		}
	} 
}


///////////////////////////////////////////
///////////// MOUSE ACTIONS ///////////////
///////////////////////////////////////////

void roll_mouseenter(t_roll *x, t_object *patcherview, t_pt pt, long modifiers) {
	x->r_ob.j_mouse_is_over = true;
    notationobj_redraw((t_notation_obj *) x);
}

void roll_mouseleave(t_roll *x, t_object *patcherview, t_pt pt, long modifiers) {
	x->r_ob.j_mouse_is_over = false;
    notationobj_redraw((t_notation_obj *) x);
}

void roll_mousemove(t_roll *x, t_object *patcherview, t_pt pt, long modifiers) {
	char redraw = false, mousepointerchanged = false;
	t_rect rect;
    
	llll_format_modifiers(&modifiers, NULL);
	jbox_get_rect_for_view(&x->r_ob.j_box.l_box.b_ob, patcherview, &rect);

	// track mouse position
	x->r_ob.j_mouse_x = pt.x; 
	x->r_ob.j_mouse_y = pt.y;
	
    if (x->r_ob.mouse_hover) {
        handle_slot_mousemove((t_notation_obj *) x, patcherview, pt, modifiers, &redraw, &mousepointerchanged);
        
        if (!x->r_ob.active_slot_notationitem && !mousepointerchanged)
            notationobj_handle_change_cursors_on_mousemove((t_notation_obj *)x, patcherview, pt, modifiers, rect);
        else if (!mousepointerchanged)
            bach_set_cursor((t_object *)x, &x->r_ob.j_mouse_cursor, patcherview, BACH_CURSOR_DEFAULT);
    }
	
	
	if (redraw)
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	else if (x->r_ob.legend > 1)
        notationobj_redraw((t_notation_obj *) x);
}



long note_get_num_breakpoints(t_note* note){
	return note->num_breakpoints;
}

// returns the x pixel of the drawn boundary (i.e. of the mouse)
// also updates the mousedown
double force_inscreen_ms_to_boundary_and_set_mouse_position(t_roll *x, double ms, t_object *patcherview, t_pt pt, char only_if_pt_is_outside_active_screen)
{
    double out = pt.x;

    if (!is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE))
        return out;

	if (!only_if_pt_is_outside_active_screen || 
		(pt.x > onset_to_xposition((t_notation_obj *)x, x->r_ob.screen_ms_end, NULL) || 
			(pt.x < onset_to_xposition((t_notation_obj *)x, x->r_ob.screen_ms_start, NULL) && x->r_ob.screen_ms_start > 0))) {
		char force_inscreen = force_inscreen_ms_to_boundary(x, ms, false, true, true, true);
		if (force_inscreen == 1)
			jmouse_setposition_box(patcherview, (t_object *) x, out = onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_end, NULL), pt.y);
		else if (force_inscreen == -1)
			jmouse_setposition_box(patcherview, (t_object *) x, out = onset_to_xposition((t_notation_obj *) x, x->r_ob.screen_ms_start, NULL), pt.y);
	}

	return out;
}



void roll_mousedrag(t_roll *x, t_object *patcherview, t_pt pt, long modifiers)
{
	char slot_dragged = 0;
	char redraw = 0; // do we have to redraw?
	char changed = 0;
	t_rect rect;
	t_pt prev_mousedrag_point = x->r_ob.j_mousedrag_point;

	llll_format_modifiers(&modifiers, NULL);

	// getting rectangle dimensions
	jbox_get_rect_for_view(&x->r_ob.j_box.l_box.b_ob, patcherview, &rect);

	x->r_ob.j_isdragging = true;

	if (x->r_ob.main_dragging_direction == 0)
		x->r_ob.main_dragging_direction = (fabs(x->r_ob.j_mousedown_point.x - pt.x) > fabs(x->r_ob.j_mousedown_point.y - pt.y) ? -1 : 1);

	
	if (modifiers & eShiftKey && modifiers & eCommandKey) {
		x->r_ob.j_mousedrag_point_shift_ffk.x += (pt.x - x->r_ob.j_mousedrag_point.x) * CONST_FINER_FROM_KEYBOARD;
		x->r_ob.j_mousedrag_point_shift_ffk.y += (pt.y - x->r_ob.j_mousedrag_point.y) * CONST_FINER_FROM_KEYBOARD;
		x->r_ob.j_mousedrag_point = pt;
	} else if (modifiers == eShiftKey) {
		if (x->r_ob.main_dragging_direction < 0)
			x->r_ob.j_mousedrag_point_shift_ffk.x = pt.x;
		else
			x->r_ob.j_mousedrag_point_shift_ffk.y = pt.y;
		x->r_ob.j_mousedrag_point = pt;
	} else {
		x->r_ob.j_mousedrag_point_shift_ffk = x->r_ob.j_mousedrag_point = pt;
	}
	
    notationobj_handle_change_cursors_on_mousedrag((t_notation_obj *)x, patcherview, pt, modifiers);
    
	// first of all: are we in a slot mode???? Cause if we are in a slot mode, we gotta handle that separately
	if (x->r_ob.active_slot_num > -1 && !is_editable((t_notation_obj *)x, k_SLOT, k_ELEMENT_ACTIONS_NONE)) return;
	slot_dragged = handle_slot_mousedrag((t_notation_obj *) x, patcherview, pt, modifiers, &changed, &redraw);

	if (redraw) {
		handle_change((t_notation_obj *) x, x->r_ob.continuously_output_changed_bang ? k_CHANGED_STANDARD_SEND_BANG : k_CHANGED_REDRAW_STATIC_LAYER, k_UNDO_OP_UNKNOWN);
		if (slot_dragged) 
			x->r_ob.changed_while_dragging = true;
		return;
	}
	if (slot_dragged)
		return; // stop! we don't care about what else could have been clicked!
	
	// dilation rectangle?
	if (x->r_ob.j_mousedown_obj_type >= k_DILATION_RECTANGLE_TOPLEFT_SQ && x->r_ob.j_mousedown_obj_type <= k_DILATION_RECTANGLE_MIDDLERIGHT_SQ) {
		if (x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_MIDDLELEFT_SQ || 
			x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_TOPLEFT_SQ ||
			x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_BOTTOMLEFT_SQ) {
			double old_width_ms = x->r_ob.dilation_rectangle.right_ms - x->r_ob.dilation_rectangle.left_ms;
			if (is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET) && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_DURATION)) {
				x->r_ob.dilation_rectangle.left_ms = MAX(0, xposition_to_onset((t_notation_obj *) x, x->r_ob.j_mousedrag_point_shift_ffk.x, 0));
                x->r_ob.dilation_rectangle.left_ms = MIN(x->r_ob.dilation_rectangle.left_ms, x->r_ob.dilation_rectangle.right_ms - 1);

                redraw = 1;
				changed = roll_sel_dilate_ms(x, (x->r_ob.dilation_rectangle.right_ms - x->r_ob.dilation_rectangle.left_ms)/old_width_ms, x->r_ob.dilation_rectangle.right_ms);
			}
		}
		if (x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_MIDDLERIGHT_SQ ||
			x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_TOPRIGHT_SQ ||
			x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_BOTTOMRIGHT_SQ) {
			double old_width_ms = x->r_ob.dilation_rectangle.right_ms - x->r_ob.dilation_rectangle.left_ms;
			if (is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET) && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_DURATION)) {
				x->r_ob.dilation_rectangle.right_ms = xposition_to_onset((t_notation_obj *) x, x->r_ob.j_mousedrag_point_shift_ffk.x, 0);
                x->r_ob.dilation_rectangle.right_ms = MAX(x->r_ob.dilation_rectangle.right_ms, x->r_ob.dilation_rectangle.left_ms + 1);
                
				redraw = 1;
				changed = roll_sel_dilate_ms(x, (x->r_ob.dilation_rectangle.right_ms - x->r_ob.dilation_rectangle.left_ms)/old_width_ms, x->r_ob.dilation_rectangle.left_ms);
			}
		}
		if (x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_TOPLEFT_SQ ||
			x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_TOPRIGHT_SQ ||
			x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_TOPMIDDLE_SQ) {
			double bottom = mc_to_yposition((t_notation_obj *) x, x->r_ob.dilation_rectangle.bottom_mc, x->r_ob.dilation_rectangle.bottom_voice);
			double old_top = mc_to_yposition((t_notation_obj *) x, x->r_ob.dilation_rectangle.top_mc, x->r_ob.dilation_rectangle.top_voice);
			double factor;
			if (is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_PITCH)) {
				x->r_ob.dilation_rectangle.top_mc = yposition_to_mc((t_notation_obj *)x, x->r_ob.j_mousedrag_point_shift_ffk.y, (t_voice *)x->r_ob.dilation_rectangle.top_voice, NULL);
				if (x->r_ob.dilation_rectangle.top_voice == x->r_ob.dilation_rectangle.bottom_voice && x->r_ob.dilation_rectangle.top_mc <= x->r_ob.dilation_rectangle.bottom_mc)
					x->r_ob.dilation_rectangle.top_mc = x->r_ob.dilation_rectangle.bottom_mc + CONST_EPSILON5;
				factor = (x->r_ob.j_mousedrag_point_shift_ffk.y - bottom)/(old_top - bottom);
				factor = MAX(factor, CONST_EPSILON5);
				redraw = 1;
				changed = roll_sel_dilate_mc(x, factor, bottom);
			}
		}
		if (x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_BOTTOMLEFT_SQ ||
			x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_BOTTOMRIGHT_SQ ||
			x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_BOTTOMMIDDLE_SQ) {
			double old_bottom = mc_to_yposition((t_notation_obj *) x, x->r_ob.dilation_rectangle.bottom_mc, x->r_ob.dilation_rectangle.bottom_voice);
			double top = mc_to_yposition((t_notation_obj *) x, x->r_ob.dilation_rectangle.top_mc, x->r_ob.dilation_rectangle.top_voice);
			double factor;
			if (is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_PITCH)) {
				x->r_ob.dilation_rectangle.bottom_mc = yposition_to_mc((t_notation_obj *)x, x->r_ob.j_mousedrag_point_shift_ffk.y, (t_voice *)x->r_ob.dilation_rectangle.bottom_voice, NULL);
				if (x->r_ob.dilation_rectangle.top_voice == x->r_ob.dilation_rectangle.bottom_voice && x->r_ob.dilation_rectangle.top_mc <= x->r_ob.dilation_rectangle.bottom_mc)
					x->r_ob.dilation_rectangle.bottom_mc = x->r_ob.dilation_rectangle.top_mc - CONST_EPSILON5;
				factor = (top - x->r_ob.j_mousedrag_point_shift_ffk.y)/(top - old_bottom);
				factor = MAX(factor, CONST_EPSILON5);
				redraw = 1;
				changed = roll_sel_dilate_mc(x, factor, top);
			}
		}
	} else if (x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_BODY) {
		double delta_ms, delta_mc;
		if (!(modifiers & eShiftKey) || x->r_ob.main_dragging_direction == -1){
			if (is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET)) {
				delta_ms = deltaxpixels_to_deltaonset((t_notation_obj *) x, x->r_ob.j_mousedrag_point_shift_ffk.x - prev_mousedrag_point.x);
				changed |= change_selection_onset(x, &delta_ms);
				x->r_ob.dilation_rectangle.left_ms += delta_ms;
				x->r_ob.dilation_rectangle.right_ms += delta_ms;
			}
		}
		if (!(modifiers & eShiftKey) || x->r_ob.main_dragging_direction == 1){
			if (is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_PITCH)) {
				delta_mc =  yposition_to_mc((t_notation_obj *)x, x->r_ob.j_mousedrag_point_shift_ffk.y, (t_voice *)x->firstvoice, NULL) - yposition_to_mc((t_notation_obj *)x, prev_mousedrag_point.y, (t_voice *)x->firstvoice, NULL);
				x->r_ob.dilation_rectangle.top_mc += delta_mc;
				x->r_ob.dilation_rectangle.bottom_mc += delta_mc;
				changed |= change_pitch_for_selection(x, delta_mc, 3, !((modifiers & eCommandKey) && (modifiers & eShiftKey)), false);
			}
		}
		redraw = 1;
	} else if (x->r_ob.j_mousedown_obj_type == k_SCROLLBAR) { // the scrollbar is being drawn!
		// new scrollbar position
		t_rect rect;
		jbox_get_rect_for_view(&x->r_ob.j_box.l_box.b_ob, patcherview, &rect);
		if (!is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE)) return;
		if (x->r_ob.j_mousedrag_point.x >= x->r_ob.j_inset_x && x->r_ob.j_mousedrag_point.x <= rect.width - x->r_ob.j_inset_x) {
			double delta_x =  pt.x - prev_mousedrag_point.x;
			if (modifiers & eShiftKey && modifiers & eCommandKey) 
				delta_x *= CONST_FINER_FROM_KEYBOARD;
			x->r_ob.hscrollbar_x += delta_x;
			redraw_hscrollbar((t_notation_obj *)x, 0);
			send_domain(x, 6, NULL);
			redraw = 0; // already redrawn in redraw_hscrollbar
			changed = 0;
		}

	} else if (x->r_ob.j_mousedown_obj_type == k_VSCROLLBAR) { // the vertical scrollbar is being drawn!
		// new vscrollbar position
		t_rect rect;
		if (!is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE)) return;
		jbox_get_rect_for_view(&x->r_ob.j_box.l_box.b_ob, patcherview, &rect);
		if (x->r_ob.j_mousedrag_point.y >= x->r_ob.j_inset_y && x->r_ob.j_mousedrag_point.y <= rect.height - x->r_ob.j_inset_y) {
			double delta_y =  pt.y - prev_mousedrag_point.y;
			if (modifiers & eShiftKey && modifiers & eCommandKey) 
				delta_y *= CONST_FINER_FROM_KEYBOARD;
			x->r_ob.vscrollbar_y += delta_y;
			redraw_vscrollbar((t_notation_obj *)x, 0);
			//			send_vdomain(x, 6, NULL);
			redraw = 0; // already redrawn in redraw_hscrollbar
			changed = 0;
		}
		
	} else if (x->r_ob.j_mousedown_obj_type == k_DRAG) { // hand grasp
			// both scrollbar position change
			t_rect rect;
			if (!is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE)) return;
			jbox_get_rect_for_view(&x->r_ob.j_box.l_box.b_ob, patcherview, &rect);
			if (x->r_ob.j_mousedrag_point.x >= x->r_ob.j_inset_x && x->r_ob.j_mousedrag_point.x <= rect.width - x->r_ob.j_inset_x && 
				x->r_ob.j_mousedrag_point.y >= x->r_ob.j_inset_y && x->r_ob.j_mousedrag_point.y <= rect.height - x->r_ob.j_inset_y) {
				double delta_x =  pt.x - prev_mousedrag_point.x;
				double delta_y =  pt.y - prev_mousedrag_point.y;
				if (modifiers & eShiftKey && modifiers & eCommandKey) {
					delta_y *= CONST_FINER_FROM_KEYBOARD;
					delta_x *= CONST_FINER_FROM_KEYBOARD;
				}
				x->r_ob.hscrollbar_x -= delta_x;
				x->r_ob.vscrollbar_y -= delta_y;
				update_hscrollbar((t_notation_obj *)x, 0);
				redraw_vscrollbar((t_notation_obj *)x, 0);
				send_domain(x, 6, NULL);
				redraw = 0; // already redrawn in redraw_hscrollbar
				changed = 0;
			}
		
	} else if (x->r_ob.j_mousedown_obj_type == k_PLAYCURSOR) { // the playcursor is being drawn!
		if (!is_editable((t_notation_obj *)x, k_PLAYCURSOR, k_ELEMENT_ACTIONS_NONE)) return;
		if (x->r_ob.playhead_dragging_mode == 1)
			x->r_ob.play_head_start_ms += deltaxpixels_to_deltaonset((t_notation_obj *) x, pt.x - prev_mousedrag_point.x);
		else
			x->r_ob.play_head_start_ms = xposition_to_onset((t_notation_obj *) x, pt.x, yposition_to_systemnumber((t_notation_obj *) x, x->r_ob.j_mousedrag_point.y));

		if (x->r_ob.play_head_start_ms < 0)
			x->r_ob.play_head_start_ms = 0;
			
        force_inscreen_ms_to_boundary_and_set_mouse_position(x, x->r_ob.play_head_start_ms, patcherview, pt, false);
		
        send_moved_playhead_position((t_notation_obj *) x, 6);
        
        redraw = 1;
		changed = 0;

	} else if (x->r_ob.j_mousedown_obj_type == k_LOOP_START || x->r_ob.j_mousedown_obj_type == k_LOOP_END) {
		double this_ms, start_ms = x->r_ob.loop_region.start.position_ms, end_ms = x->r_ob.loop_region.end.position_ms;
		
		if (!is_editable((t_notation_obj *)x, k_LOOP_REGION, k_ELEMENT_ACTIONS_NONE)) return;
        
        create_header_undo_tick((t_notation_obj *)x, k_HEADER_LOOP);

		this_ms = xposition_to_onset((t_notation_obj *) x, pt.x, yposition_to_systemnumber((t_notation_obj *) x, x->r_ob.j_mousedrag_point.y));
		this_ms = MAX(this_ms, 0);
		
		if (modifiers & eShiftKey) {
			t_chord *nearest_chord = get_nearest_chord(x, this_ms);
			if (nearest_chord)
				this_ms = nearest_chord->onset;
		}
		
		force_inscreen_ms_to_boundary_and_set_mouse_position(x, this_ms, patcherview, pt, false);
		
		if (x->r_ob.j_mousedown_obj_type == k_LOOP_START)
			start_ms = this_ms;
		else if (x->r_ob.j_mousedown_obj_type == k_LOOP_END)
			end_ms = this_ms;
			
		if (start_ms > end_ms) {
			swap_doubles(&start_ms, &end_ms);
			set_mousedown((t_notation_obj *) x, WHITENULL_llll, x->r_ob.j_mousedown_obj_type == k_LOOP_START ? k_LOOP_END : k_LOOP_START);
		}
		
		set_loop_region_from_extremes(x, start_ms, end_ms);
		send_loop_region((t_notation_obj *) x, 6);
		redraw = 1;
		changed = 0;

	} else if (x->r_ob.j_mousedown_obj_type == k_LOOP_REGION) {
		double start_ms = x->r_ob.loop_region.start.position_ms, end_ms = x->r_ob.loop_region.end.position_ms;
		double delta_x = pt.x - x->r_ob.floatdragging_x;
		double delta_ms = deltaxpixels_to_deltaonset((t_notation_obj *)x, delta_x);
	 	double new_start_ms, new_end_ms;
		if (!is_editable((t_notation_obj *)x, k_LOOP_REGION, k_ELEMENT_ACTIONS_NONE)) return;
		
        create_header_undo_tick((t_notation_obj *)x, k_HEADER_LOOP);

		if (delta_ms < 0 && -delta_ms > start_ms)
			delta_ms = -start_ms;
			
		new_start_ms = start_ms + delta_ms;
		new_end_ms = end_ms + delta_ms;

		double fl = force_inscreen_ms_to_boundary_and_set_mouse_position(x, delta_x > 0 ? new_end_ms : new_start_ms, patcherview, pt, false);
		
		x->r_ob.floatdragging_x = fl;
		set_loop_region_from_extremes(x, new_start_ms, new_end_ms);
		send_loop_region((t_notation_obj *) x, 6);
		redraw = 1;
		changed = 0;
		
	} else if (x->r_ob.j_mousedown_obj_type == k_VOICE && x->r_ob.j_mousedown_ptr && (modifiers & eShiftKey)) {
		if (!notation_item_is_globally_locked((t_notation_obj *) x, (t_notation_item *)x->r_ob.j_mousedown_ptr)){
			long number = ((t_rollvoice *)x->r_ob.j_mousedown_ptr)->v_ob.number;
			double delta_y =  pt.y - prev_mousedrag_point.y;
			if (modifiers & eControlKey) number++;
			
			if (!is_editable((t_notation_obj *)x, k_VOICE, k_MODIFICATION_POSITION)) return;
			if (number == 0) {
				x->r_ob.voiceuspacing_as_floatlist[0] += delta_y/x->r_ob.zoom_y;
				x->r_ob.head_vertical_additional_uspace += delta_y/x->r_ob.zoom_y;
			} else if (number < x->r_ob.num_voices + 1) {
				t_rollvoice *voice = nth_rollvoice(x, number-1);
                voice = (t_rollvoice *)voiceensemble_get_lastvoice((t_notation_obj *)x, (t_voice *)voice);
				if (voice) {
					x->r_ob.voiceuspacing_as_floatlist[number] += delta_y/x->r_ob.zoom_y;
					voice->v_ob.vertical_uspacing += delta_y/x->r_ob.zoom_y;
					if (modifiers & eAltKey && voice->next && number < CONST_MAX_VOICES){
						x->r_ob.voiceuspacing_as_floatlist[number + 1] -= delta_y/x->r_ob.zoom_y;
						voice->next->v_ob.vertical_uspacing -= delta_y/x->r_ob.zoom_y;
					}
				}
			}
			if (x->r_ob.link_vzoom_to_height)
				auto_set_rectangle_size((t_notation_obj *) x);
			else
				calculate_voice_offsets((t_notation_obj *) x);
            reset_all_articulations_positions((t_notation_obj *) x);
			redraw = 1;
			changed = 0;
		}
	} else if (x->r_ob.j_mousedown_obj_type == k_REGION) { // a region has been selected
		if (!is_editable((t_notation_obj *)x, k_SELECTION, k_SINGLE_SELECTION) && !is_editable((t_notation_obj *)x, k_SELECTION, k_MULTIPLE_SELECTION)) return;
		x->r_ob.j_selected_region_ms1 = xposition_to_onset((t_notation_obj *) x, x->r_ob.j_mousedown_point.x, yposition_to_systemnumber((t_notation_obj *) x, x->r_ob.j_mousedrag_point.y));
		x->r_ob.j_selected_region_mc1 = yposition_to_mc((t_notation_obj *)x, x->r_ob.j_mousedown_point.y, NULL, NULL);
		x->r_ob.j_selected_region_ms2 = xposition_to_onset((t_notation_obj *) x, pt.x, yposition_to_systemnumber((t_notation_obj *) x, pt.y));
		x->r_ob.j_selected_region_mc2 = yposition_to_mc((t_notation_obj *)x, pt.y, NULL, NULL);
		
        if (x->r_ob.j_selected_region_ms1 > x->r_ob.j_selected_region_ms2)
            swap_doubles(&x->r_ob.j_selected_region_ms1, &x->r_ob.j_selected_region_ms2);
		
        if (x->r_ob.j_mousedown_point.y < pt.y) {
            swap_doubles(&x->r_ob.j_selected_region_mc1, &x->r_ob.j_selected_region_mc2);
            x->r_ob.j_selected_region_voice2 = yposition_to_voicenumber((t_notation_obj *)x, x->r_ob.j_mousedown_point.y, -1, k_VOICEENSEMBLE_INTERFACE_FIRST);
            x->r_ob.j_selected_region_voice1 = yposition_to_voicenumber((t_notation_obj *)x, pt.y, -1, k_VOICEENSEMBLE_INTERFACE_LAST);
        } else {
            x->r_ob.j_selected_region_voice1 = yposition_to_voicenumber((t_notation_obj *)x, x->r_ob.j_mousedown_point.y, -1, k_VOICEENSEMBLE_INTERFACE_LAST);
            x->r_ob.j_selected_region_voice2 = yposition_to_voicenumber((t_notation_obj *)x, pt.y, -1, k_VOICEENSEMBLE_INTERFACE_FIRST);
        }
        
        
		lock_general_mutex((t_notation_obj *)x);
		clear_preselection((t_notation_obj *)x);
		preselect_elements_in_region_for_mouse_selection(x, x->r_ob.j_selected_region_ms1, x->r_ob.j_selected_region_ms2, x->r_ob.j_selected_region_mc1, x->r_ob.j_selected_region_mc2, x->r_ob.j_selected_region_voice1, x->r_ob.j_selected_region_voice2);
		if ((x->r_ob.j_mousedown_point.y > 3 * x->r_ob.zoom_y && 3 * x->r_ob.zoom_y > pt.y) || (pt.y > 3 * x->r_ob.zoom_y && 3 * x->r_ob.zoom_y > x->r_ob.j_mousedown_point.y))
			preselect_markers_in_region((t_notation_obj *)x, x->r_ob.j_selected_region_ms1, x->r_ob.j_selected_region_ms2);
		if (!is_editable((t_notation_obj *)x, k_SELECTION, k_MULTIPLE_SELECTION)) {
			if ((x->r_ob.num_selecteditems > 0 && x->r_ob.firstpreselecteditem) || 
				(x->r_ob.num_selecteditems == 0 && x->r_ob.firstpreselecteditem != x->r_ob.lastpreselecteditem))
				clear_preselection((t_notation_obj *)x);
		}
		unlock_general_mutex((t_notation_obj *)x);
		
		if (is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE)) {
			double screen_end_x = onset_to_xposition((t_notation_obj *)x, x->r_ob.screen_ms_end, NULL);
			force_inscreen_ms_to_boundary_and_set_mouse_position(x, pt.x > screen_end_x ? x->r_ob.j_selected_region_ms2 : x->r_ob.j_selected_region_ms1, patcherview, pt, true);
		}
		
		redraw = 1;
		changed = 0;
	} else if (x->r_ob.j_mousedown_obj_type == k_ZOOMING_REGION) {
		redraw = 1;
		changed = 0;
	} else if (x->r_ob.j_mousedown_obj_type > 0) { // if the mousedown is on a musical object
		
		// depends on selection
		x->r_ob.j_selection_hasbeendragged = 1;
        
        if (x->r_ob.j_mousedown_obj_type == k_DURATION_LINE && modifiers == eControlKey) { // slope change
                double delta_slope = (x->r_ob.floatdragging_y - pt.y)/(CONST_SLOPE_DRAG_UCHANGE * x->r_ob.zoom_y);
                if (!is_editable((t_notation_obj *)x, k_PITCH_BREAKPOINT, k_MODIFICATION_GENERIC)) return;
                if (modifiers & eShiftKey && modifiers & eCommandKey)
                    delta_slope *= CONST_FINER_FROM_KEYBOARD;
                if (x->r_ob.allow_glissandi) {
                    // retrieve correct bpt
                    t_note *temp_note = ((t_duration_line *)x->r_ob.j_mousedown_ptr)->owner;
                    t_bpt *temp_bpt, *bpt = NULL;
                    double click_onset = xposition_to_onset((t_notation_obj *)x, x->r_ob.j_mousedown_point.x, 0);
                    for (temp_bpt = temp_note->firstbreakpoint ? temp_note->firstbreakpoint->next : NULL; temp_bpt; temp_bpt = temp_bpt->next) {
                        if (notation_item_get_onset_ms((t_notation_obj *)x, (t_notation_item *)temp_bpt) > click_onset) {
                            bpt = temp_bpt;
                            break;
                        }
                    }
                    if (bpt)
                        change_breakpoint_slope((t_notation_obj *)x, bpt, fabs(delta_slope), fsign(delta_slope));
                    changed = 1;
                    redraw = 1;
                }
                x->r_ob.floatdragging_y = pt.y;
                if (changed)
                    x->r_ob.changed_while_dragging = true;
            
        } else if (x->r_ob.selection_type == k_PITCH_BREAKPOINT) {
		
			if (modifiers == eControlKey && x->r_ob.breakpoints_have_velocity) { // velocity change
				double delta_vel = (x->r_ob.floatdragging_y - pt.y)/(CONST_VELOCITY_DRAG_UCHANGE * x->r_ob.zoom_y);			
				if (!is_editable((t_notation_obj *)x, k_PITCH_BREAKPOINT, k_MODIFICATION_VELOCITY)) return;
				if (modifiers & eShiftKey && modifiers & eCommandKey) 
					delta_vel *= CONST_FINER_FROM_KEYBOARD;
				changed = change_selection_velocity((t_notation_obj *) x, delta_vel);
				if (changed) 
					x->r_ob.changed_while_dragging = true;
				x->r_ob.floatdragging_y = pt.y;
				redraw = 1;

			} else {
				long system = 1; // used for yposition_to_mc
				char can_change_mc = true, can_change_onset = true;
				double delta_y;

				if (modifiers & eShiftKey) {
					if (x->r_ob.j_dragging_direction == 0)
						decide_dragging_direction((t_notation_obj *) x, pt);
					
					if (x->r_ob.j_dragging_direction == 1)
						can_change_mc = false;
					else if (x->r_ob.j_dragging_direction == -1)
						can_change_onset = false;
				} 

				can_change_onset &= is_editable((t_notation_obj *)x, k_PITCH_BREAKPOINT, k_MODIFICATION_ONSET);
				can_change_mc &= is_editable((t_notation_obj *)x, k_PITCH_BREAKPOINT, k_MODIFICATION_PITCH);
				
				if (only_tails_are_selected((t_notation_obj *) x)){
					// new duration tail
					can_change_onset &= is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_DURATION);
					double delta_length_ms = (xposition_to_onset((t_notation_obj *) x, pt.x, 0) - xposition_to_onset((t_notation_obj *) x, x->r_ob.floatdragging_x, 0));

					if (modifiers & eShiftKey && modifiers & eCommandKey) 
						delta_length_ms *= CONST_FINER_FROM_KEYBOARD;
					
					if (can_change_onset) {
						if (modifiers & eShiftKey && modifiers & eControlKey)
							trim_selection_end((t_notation_obj *) x, delta_length_ms);
						else
							change_selection_duration(x, delta_length_ms);
					}
					
					if (x->r_ob.allow_glissandi && can_change_mc) {
						delta_y = yposition_to_mc((t_notation_obj *)x, pt.y, (t_voice *)x->firstvoice, &system) - yposition_to_mc((t_notation_obj *)x, x->r_ob.floatdragging_y, (t_voice *)x->firstvoice, &system); // if Shift, no vertical movement allowed
						if (modifiers & eShiftKey && modifiers & eCommandKey) 
							delta_y *= CONST_FINER_FROM_KEYBOARD;
						move_selection_breakpoint(x, 0., delta_y, 1.);
						if (x->r_ob.breakpoints_have_noteheads && x->r_ob.snap_pitch_to_grid_when_editing)
							snap_pitch_to_grid_for_selection((t_notation_obj *)x);
						changed = 1;
					}
				} else {
					double delta_x = pt.x - x->r_ob.floatdragging_x;
					delta_y = yposition_to_mc((t_notation_obj *)x, pt.y, (t_voice *)x->firstvoice, &system) - yposition_to_mc((t_notation_obj *)x, x->r_ob.floatdragging_y, (t_voice *)x->firstvoice, &system);
					if (modifiers & eShiftKey && modifiers & eCommandKey) {
						delta_y *= CONST_FINER_FROM_KEYBOARD;
						delta_x *= CONST_FINER_FROM_KEYBOARD;
					}
					
					move_selection_breakpoint(x, delta_x * can_change_onset, delta_y * can_change_mc, 0);				

					if (x->r_ob.breakpoints_have_noteheads && x->r_ob.snap_pitch_to_grid_when_editing)
						snap_pitch_to_grid_for_selection((t_notation_obj *)x);
				}
				changed = 1;
			}
			
			if (changed) 
				x->r_ob.changed_while_dragging = true;
			x->r_ob.floatdragging_x = pt.x;	
			x->r_ob.floatdragging_y = pt.y;
			redraw = 1;
		
		} else if (x->r_ob.selection_type == k_MARKER && x->r_ob.j_mousedown_obj_type == k_MARKER) { // only markers are selected
			double delta_onset = deltaxpixels_to_deltaonset((t_notation_obj *)x, pt.x - x->r_ob.floatdragging_x);

			if (!is_editable((t_notation_obj *)x, k_MARKER, k_MODIFICATION_ONSET)) return;

			if (!(x->r_ob.header_undo_flags & k_HEADER_MARKERS)) {
				create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
				x->r_ob.header_undo_flags |= k_HEADER_MARKERS;
			}

			if (modifiers & eShiftKey && modifiers & eCommandKey) 
				delta_onset *= CONST_FINER_FROM_KEYBOARD;

			if (modifiers & eAltKey && !(modifiers & eCommandKey) && !x->r_ob.j_mousedrag_copy_ptr && is_editable((t_notation_obj *)x, k_MARKER, k_CREATION)) { // copy it
				t_notation_item *temp;
				lock_markers_mutex((t_notation_obj *)x);
				lock_general_mutex((t_notation_obj *)x);
				for (temp = x->r_ob.firstselecteditem; temp; temp = temp->next_selected) {
					if (temp->type == k_MARKER) {
						double marker_ms = ((t_marker *) temp)->position_ms;
						t_llll *content = NULL;
						t_marker *newmarker;
						
						create_header_undo_tick((t_notation_obj *) x, k_HEADER_MARKERS);
						if (((t_marker *)temp)->content) {
							content = llll_clone(((t_marker *)temp)->content);
						}
						t_llll *chosen = make_marker_name_unique((t_notation_obj *) x, ((t_marker *) temp)->r_it.names);
						newmarker = add_marker((t_notation_obj *) x, chosen, marker_ms + delta_onset, build_timepoint(0, long2rat(0)), k_MARKER_ATTACH_TO_MS, ((t_marker *) temp)->role, content, 0);
						notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)newmarker);
						if (temp == x->r_ob.j_mousedown_ptr) {
							set_mousedown((t_notation_obj *) x, newmarker, k_MARKER);
							x->r_ob.j_mousedrag_copy_ptr = newmarker;
						}
                        llll_free(chosen);
					}
				}
                
                // resetting mousedown differences
                double mousedown_marker_ms = ((t_marker *)x->r_ob.j_mousedown_ptr)->position_ms;
                double marker_mousedown_ux = ms_to_unscaled_xposition((t_notation_obj *)x, mousedown_marker_ms, 0);
                for (temp = x->r_ob.firstpreselecteditem; temp; temp = temp->next_preselected) {
                    if (temp->type == k_MARKER) {
                        double this_ux = get_marker_ux_position((t_notation_obj *)x, (t_marker *)temp);
                        ((t_marker *)temp)->ux_difference_with_mousedown_marker = this_ux - marker_mousedown_ux;
                    }
                }
                    
				clear_selection((t_notation_obj *) x);
				move_preselecteditems_to_selection((t_notation_obj *) x, k_SELECTION_MODE_INVOLUTIVE, false, false);
				unlock_general_mutex((t_notation_obj *)x);
				unlock_markers_mutex((t_notation_obj *)x);;
			}
			
            double delta_ms = 0;
            double mousedown_ux = xposition_to_unscaled_xposition((t_notation_obj *)x, (modifiers & eShiftKey && modifiers & eCommandKey) ? x->r_ob.j_mousedrag_point_shift_ffk.x : pt.x) - x->r_ob.ux_click_marker_diff;
            double mousedown_marker_ms = ((t_marker *)x->r_ob.j_mousedown_ptr)->position_ms;
//            dev_post("----------------------------------");
//            dev_post("--- mousedown_ux, ms: %.2f, %.2fms", mousedown_ux, unscaled_xposition_to_ms((t_notation_obj *)x, mousedown_ux, 0));
			changed = move_selected_ms_attached_markers((t_notation_obj *) x, mousedown_ux, (modifiers & eShiftKey && !(modifiers & eCommandKey)), &delta_ms);
            
            if (modifiers == eControlKey) {
                t_rollvoice *voice;
                t_chord *chord;
                t_marker *mk;
                double mousedown_ms = unscaled_xposition_to_ms((t_notation_obj *)x, mousedown_ux, 0);
//                dev_post("--- mousedown_ms: %.2f, delta_ms: %.2f", mousedown_ms, delta_ms);
                for (voice = (t_rollvoice *)x->r_ob.firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
                    for (chord = voice->firstchord; chord; chord = chord->next) {
//                        dev_post("--- chord_onset: %.2f, diff: %.2f", chord->onset, chord->onset - mousedown_ms);
                        if (!notation_item_is_selected((t_notation_obj *)x, (t_notation_item *) chord) && chord->onset >= mousedown_marker_ms &&
                            !notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *) chord)) {
                            create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)chord, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
                            chord->onset += delta_ms;
                            chord->r_it.flags |= k_FLAG_TO_BE_SNAPPED;
                        }
                    }
                    check_chords_order_for_voice(x, voice);
                }
                for (mk = x->r_ob.firstmarker; mk; mk = mk->next)
                    if (!notation_item_is_selected((t_notation_obj *)x, (t_notation_item *) mk) && mk->position_ms >= mousedown_marker_ms &&
                        !notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *) mk)) {
                        mk->position_ms += delta_ms;
                        mk->r_it.flags |= k_FLAG_TO_BE_SNAPPED;
                    }
                
                x->r_ob.need_snap_some_nonselected_items = true;
            }

			x->r_ob.floatdragging_x = force_inscreen_ms_to_boundary_and_set_mouse_position(x, xposition_to_onset((t_notation_obj *)x, pt.x, NULL), patcherview, pt, true);
			x->r_ob.floatdragging_y = pt.y;

			recompute_total_length((t_notation_obj *)x);
			redraw = changed;

		} else { // mixed selection, or just notes, or
		
			char can_change_onset = 1;
			char can_change_mc = 1;
			char can_change_vel = 0;
            
            char trim_start = (modifiers & eShiftKey && modifiers & eControlKey && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET) && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_DURATION));
			
			// modifiers: what can I change by dragging the note?
			if (modifiers & eShiftKey) {
                if (trim_start)
                    x->r_ob.j_dragging_direction = 1;
				else if (x->r_ob.j_dragging_direction == 0)
					decide_dragging_direction((t_notation_obj *) x, pt);
					
				if (x->r_ob.j_dragging_direction == 1)
					can_change_mc = 0;
				else if (x->r_ob.j_dragging_direction == -1)
					can_change_onset = 0;
			}

			if (modifiers == eControlKey){ // eControlKey it is used to change the velocities!
				can_change_mc = 0;
				can_change_onset = 0;
				can_change_vel = 1;
			}
			
			can_change_onset &= is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET);
			can_change_mc &= is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_PITCH);
			can_change_vel &= is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_VELOCITY);
			
			if (modifiers & eAltKey && !(modifiers & eCommandKey) && !x->r_ob.j_mousedrag_copy_ptr) { // eAltKey is to quickly copy a note!!!
				long copy_mode;
				t_notation_item *temp;
				can_change_mc = 0;
				can_change_onset = 0;
				can_change_vel = 0;
				
				// what are we supposed to do? each copied item is
				if (modifiers & eShiftKey && x->r_ob.j_dragging_direction == -1) 
					copy_mode = 1; // if we press Shift and drag vertically, we copy the NOTES in the same chords!!
				else
					copy_mode = 0; // copy to new chords
				
				if (is_editable((t_notation_obj *)x, copy_mode ? k_NOTE : k_CHORD, k_CREATION)) {
					
					lock_general_mutex((t_notation_obj *) x);	
					clear_preselection((t_notation_obj *) x); // we clear the preselection and put - one by one - the items in the selection.
					
					// we gotta copy _all_ the selected items
					temp = x->r_ob.firstselecteditem;
					while (temp) {
						x->r_ob.floatdragging_y = pt.y;	
						x->r_ob.floatdragging_x = pt.x;
						if (temp->type == k_NOTE) { // it's a note
							if (copy_mode == 1) { // add the note(s) to the SAME chord
								if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)((t_note *)temp)->parent)) {
									t_note *new_note = clone_note((t_notation_obj *) x, (t_note *)temp, k_CLONE_FOR_SAME_CHORD); // we clone the note
									
									if (!(((t_note *)temp)->parent->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
										create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)(((t_note *)temp)->parent), k_CHORD, k_UNDO_MODIFICATION_CHANGE);
									
									insert_note((t_notation_obj *) x, ((t_note *)temp)->parent, new_note, 0);
									((t_note *)temp)->parent->need_recompute_parameters = true; // we have to recalculate chord parameters 
									x->r_ob.j_mousedrag_copy_ptr = new_note;
									// checking if we have to transfer the mousedown pointer
									if (temp == x->r_ob.j_mousedown_ptr || ((t_note *)temp)->parent == x->r_ob.j_mousedown_ptr) {
										set_mousedown((t_notation_obj *) x, new_note, k_NOTE);
									}
									changed = 1;
									if (changed) 
										x->r_ob.changed_while_dragging = true;
									notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)new_note);
								}
							} else { // copy it to a new chord
								t_notation_item *selitem;
								t_chord *new_chord = clone_selected_notes_into_chord((t_notation_obj *) x, ((t_note *)temp)->parent, k_CLONE_FOR_NEW); // we clone the selected notes into a new chord
								insert_chord(x, ((t_note *)temp)->parent->voiceparent, new_chord, 0);
								
								create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)new_chord, k_UNDO_MODIFICATION_DELETE);
								
								x->r_ob.j_mousedrag_copy_ptr = new_chord;
								// checking if we have to transfer the mousedown pointer
								if (temp == x->r_ob.j_mousedown_ptr) {
									set_mousedown((t_notation_obj *) x, new_chord, k_CHORD);
								} else {
									t_chord *oldchord = ((t_note *)temp)->parent;
									t_note *nt1, *nt2;
									for (nt1 = oldchord->firstnote, nt2 = new_chord->firstnote; nt1 && nt2; nt1 = nt1->next, nt2 = nt2->next)
										if (nt1 == x->r_ob.j_mousedown_ptr) {
											set_mousedown((t_notation_obj *) x, nt2, k_NOTE);
										}
								}
								notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)new_chord);
								// remove all the notes of the chord from the selection (otherwise, we would clone the chord n times)
								selitem = temp->next_selected;
								while (selitem) {
									t_notation_item *next = selitem->next_selected;
									if (selitem->type == k_NOTE && ((t_note *)selitem)->parent == ((t_note *)temp)->parent)
										notation_item_delete_from_selection((t_notation_obj *) x, selitem);
									selitem = next;
								}
								changed = 1;
								if (changed) 
									x->r_ob.changed_while_dragging = true;
							}
						} else if (temp->type == k_CHORD) { // it's a chord
							if (copy_mode != 1 || !notation_item_is_globally_locked((t_notation_obj *) x, temp)) {
                                t_chord *new_chord = clone_chord((t_notation_obj *) x, (t_chord *)temp, copy_mode == 1 ? k_CLONE_FOR_SAME_CHORD : k_CLONE_FOR_NEW);
								
								if (copy_mode == 1) { // it's a chord, but still we have to copy each note WITHIN the chord: so we MERGE it!
									t_note *new_mousedown_note;
									
									if (!(((t_chord *)temp)->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
										create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, temp, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
									
									// merge the chords
									new_mousedown_note = merge_chords(x, (t_chord *)temp, new_chord, true, true, true);
									((t_chord *)temp)->need_recompute_parameters = true; // we have to recalculate chord parameters 
									x->r_ob.j_mousedrag_copy_ptr = ((t_chord *)temp);
									if (new_mousedown_note) {
										set_mousedown((t_notation_obj *) x, new_mousedown_note, k_NOTE);
										//								} else {
										//									x->r_ob.j_mousedown_ptr = ((t_chord *)temp->item);
										//									x->r_ob.j_mousedown_obj_type = k_CHORD;
									}
								} else { // copy it to a new chord
									// just insert chord in the list
									t_note *nt1, *nt2;
									t_chord *oldchord = ((t_chord *)temp);
									insert_chord(x, oldchord->voiceparent, new_chord, 0);
									
									create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)new_chord, k_UNDO_MODIFICATION_DELETE);
									
									x->r_ob.j_mousedrag_copy_ptr = new_chord;
									if ((oldchord->num_notes == 1 && x->r_ob.j_mousedown_ptr == oldchord) || x->r_ob.j_mousedown_ptr == oldchord->firstnote) {
										set_mousedown((t_notation_obj *) x, (t_notation_item *)new_chord, k_CHORD);
									} else {
										for (nt1 = oldchord->firstnote, nt2 = new_chord->firstnote; nt1 && nt2; nt1 = nt1->next, nt2 = nt2->next)
											if (nt1 == x->r_ob.j_mousedown_ptr) {
												set_mousedown((t_notation_obj *) x, (t_notation_item *)nt2, k_NOTE);
											}
									}
									notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)new_chord);
								}
								changed = 1;
								if (changed) 
									x->r_ob.changed_while_dragging = true;
							}
//						} else if (temp->type == k_NOTE) {
							
						}
						temp = temp->next_selected;
					}
					clear_selection((t_notation_obj *) x);
					move_preselecteditems_to_selection((t_notation_obj *) x, k_SELECTION_MODE_INVOLUTIVE, false, false);
					unlock_general_mutex((t_notation_obj *)x);	
					
					redraw = 1;
				}
			}
			
            if (trim_start) {
                double delta_onset = xposition_to_onset((t_notation_obj *) x, pt.x, 0) - xposition_to_onset((t_notation_obj *) x, x->r_ob.floatdragging_x, 0);
                if (modifiers & eShiftKey && modifiers & eCommandKey)
                    delta_onset *= CONST_FINER_FROM_KEYBOARD;
                changed = trim_selection_start((t_notation_obj *)x, delta_onset);
                
                if (changed)
                    x->r_ob.changed_while_dragging = true;
                
                x->r_ob.floatdragging_x = force_inscreen_ms_to_boundary_and_set_mouse_position(x, xposition_to_onset((t_notation_obj *)x, pt.x, NULL), patcherview, pt, true);
                
                redraw = 1;
            } else {
                
                if (can_change_onset){
                    // new onset
                    double delta_onset = xposition_to_onset((t_notation_obj *) x, pt.x, 0) - xposition_to_onset((t_notation_obj *) x, x->r_ob.floatdragging_x, 0);
                    if (modifiers & eShiftKey && modifiers & eCommandKey)
                        delta_onset *= CONST_FINER_FROM_KEYBOARD;
                    changed = change_selection_onset(x, &delta_onset);
                    
                    if (changed)
                        x->r_ob.changed_while_dragging = true;
                    
                    x->r_ob.floatdragging_x = force_inscreen_ms_to_boundary_and_set_mouse_position(x, xposition_to_onset((t_notation_obj *)x, pt.x, NULL), patcherview, pt, true);
                    
                    redraw = 1;
                }
                
                if (can_change_mc){
                    // new midicents
                    long system = -1; // auto
                    double delta_mc = 0;
                    if ( ((x->r_ob.j_mousedown_obj_type == k_NOTE && x->r_ob.j_mousedown_ptr) || (x->r_ob.j_mousedown_obj_type == k_CHORD && x->r_ob.j_mousedown_ptr)) &&
                        !(modifiers & eShiftKey  &&  modifiers & eCommandKey  &&  !x->r_ob.snap_pitch_to_grid_when_editing)) {
                        
                        if (pt.y >= 0 && pt.y <= rect.height) { // point is in the window
                            t_note *nt = (x->r_ob.j_mousedown_obj_type == k_NOTE) ? (t_note *) x->r_ob.j_mousedown_ptr : ((t_chord *) x->r_ob.j_mousedown_ptr)->firstnote;
                            delta_mc = yposition_to_mc((t_notation_obj *)x, pt.y, (t_voice *)(t_voice *)nt->parent->voiceparent, &system) - nt->midicents;
                        }
                        
                    } else {
                        delta_mc = yposition_to_mc((t_notation_obj *)x, pt.y, (t_voice *)x->firstvoice, &system) - yposition_to_mc((t_notation_obj *)x, x->r_ob.floatdragging_y, (t_voice *)x->firstvoice, &system);
                    }
                    if (modifiers & eShiftKey && modifiers & eCommandKey && !x->r_ob.snap_pitch_to_grid_when_editing)
                        delta_mc *= CONST_FINER_FROM_KEYBOARD;
                    changed = change_pitch_for_selection(x, delta_mc, 3, !((modifiers & eCommandKey) && (modifiers & eShiftKey)), false);
                    if (changed)
                        x->r_ob.changed_while_dragging = true;
                    x->r_ob.floatdragging_y = pt.y;
                    redraw = 1;
                }
                
                if (can_change_vel){
                    // new velocity
                    double delta_vel = (x->r_ob.floatdragging_y - pt.y)/(CONST_VELOCITY_DRAG_UCHANGE * x->r_ob.zoom_y);				
                    if (modifiers & eShiftKey && modifiers & eCommandKey) 
                        delta_vel *= CONST_FINER_FROM_KEYBOARD;
                    changed = change_selection_velocity((t_notation_obj *) x, delta_vel);
                    if (changed) {
                        x->r_ob.changed_while_dragging = true;
                    }
                    x->r_ob.floatdragging_y = pt.y;
                    redraw = 1;
                }
            }
		}
	}
	
	x->r_ob.j_mouse_hasbeendragged = 1; // mouse has been dragged
	
	if (redraw) {
		invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
		if (changed && x->r_ob.j_mouse_is_down) {
			x->r_ob.changed_while_dragging = true;
			handle_change((t_notation_obj *) x, x->r_ob.continuously_output_changed_bang ? k_CHANGED_STANDARD_SEND_BANG : k_CHANGED_REDRAW_STATIC_LAYER, k_UNDO_OP_UNKNOWN);
		}
	}
	
}


t_chord *shift_note_allow_voice_change(t_roll *x, t_note *note, double delta, char mode, char *old_chord_deleted, char allow_voice_change)
{
// mode == 0: delta are the delta-steps on the scale (e.g. from C3 to F3, it is 5 steps in the semitone scale, 10 steps in the quartertonal scale)
// mode == 1: delta is delta_midicents
// returns a pointer to the new chord, if the chord has changed.
// also changes note to a new note if note has changed voice
// fills "old_chord_deleted" with true if the old chord was deleted, false otherwise

	double note_y_real;
	long note_new_voice, note_old_system, note_new_system;
	t_chord *newch = NULL; 
	double prev_mc = note->midicents; // mc before change
	
	if (old_chord_deleted) 
		*old_chord_deleted = false;

	note_old_system = onset_to_system_index((t_notation_obj *) x, note->parent->onset);
	if (mode == 0) { // snapped to grid
		note->midicents = get_next_step_depending_on_editing_ranges((t_notation_obj *)x, note->midicents, note->parent->voiceparent->v_ob.number, delta);
	} else
		note->midicents += delta;
	
	note_y_real = note_old_system * x->r_ob.system_jump + mc_to_yposition((t_notation_obj *)x, note->midicents, (t_voice *) note->parent->voiceparent);
	note_new_system = yposition_to_systemnumber((t_notation_obj *) x, note_y_real);
	note_new_voice = allow_voice_change ? yposition_to_voicenumber((t_notation_obj *)x, note_y_real, note_new_system, k_VOICEENSEMBLE_INTERFACE_ACTIVE) : note->parent->voiceparent->v_ob.number;

	if (note_new_voice < 0 || // active voice not found; let's keep this one
        (note_new_voice == note->parent->voiceparent->v_ob.number && note_new_system == note_old_system) ||	// if the legitimate voice is the same of the previous voice...
        do_voices_belong_to_same_voiceensemble((t_notation_obj *) x, (t_voice *)nth_rollvoice(x, note_new_voice), (t_voice *)note->parent->voiceparent) || // if the two voices belong to the same ensemble
		((note->midicents - prev_mc) * (note_new_voice - note->parent->voiceparent->v_ob.number + x->r_ob.num_voices * (note_new_system - note_old_system)) >= 0)) { // ...or if the voice movement is not in phase with the mc movement (e.g. i'm dragging upwards a very low note on a staff: i don't want it to go to the lower staff!)
        
		note_set_auto_enharmonicity(note); // automatic accidentals for retranscribing!
		constraint_midicents_depending_on_editing_ranges((t_notation_obj *)x, &note->midicents, note_new_voice); 
        update_all_accidentals_for_chord_if_needed(x, note->parent);
    
    } else { // note is changing voice!
		double threshold_y, change_mc1, change_mc2;
		t_note *note_in_new_voice;
		t_chord *temp_ch, *chord_for_note_insertion;
		t_rollvoice *new_voice = nth_rollvoice(x, note_new_voice);
//		post("voices: FROM %d TO %d", note->parent->voiceparent->v_ob.number, new_voice->v_ob.number);
		// new midicents?
//		double new_mc = note->midicents

		note_in_new_voice = clone_note((t_notation_obj *)x, note, k_CLONE_FOR_ORIGINAL); // we clone the note
		note->midicents -= (mode == 0) ? delta * (200. / x->r_ob.tone_division) : delta; // step back
		threshold_y = (new_voice->v_ob.number > note->parent->voiceparent->v_ob.number) ? 
						new_voice->v_ob.offset_y + CONST_VOICE_THRESHOLD * x->r_ob.zoom_y :
						note->parent->voiceparent->v_ob.offset_y + CONST_VOICE_THRESHOLD * x->r_ob.zoom_y;
		change_mc1 = yposition_to_mc((t_notation_obj *)x, threshold_y, (t_voice *)new_voice, &note_new_system);
		change_mc2 = yposition_to_mc((t_notation_obj *)x, threshold_y, (t_voice *)note->parent->voiceparent, &note_old_system);
//		double difference = change_mc1 - change_mc2;
//		post("diff = %f", difference);
//		note_in_new_voice->midicents = yposition_to_mc(x, note_y_real, new_voice);
//		note_in_new_voice->midicents = change_mc1 - ((-delta) - (note->midicents - change_mc2));
		note_in_new_voice->midicents = change_mc1 - ((mode == 0 ? -delta * (200. / x->r_ob.tone_division) : -delta) - (note->midicents - change_mc2));

		constraint_midicents_depending_on_editing_ranges((t_notation_obj *)x, &note_in_new_voice->midicents, note_new_voice); 


		// we look if there's a chord EXACTLY with the same onset, we add the note to the chord!
		temp_ch = new_voice->firstchord;
		chord_for_note_insertion = NULL;
		while (temp_ch) {
			if (temp_ch->onset == note->parent->onset + (note_new_system - note_old_system) * x->r_ob.ms_on_a_line) {
				chord_for_note_insertion = temp_ch;
				break;
			} else if (temp_ch->onset >= note->parent->onset) {
				break;
			}
			temp_ch = temp_ch->next; 
		}

		note_set_auto_enharmonicity(note_in_new_voice); // accidental will be automatically chosen!
		note_in_new_voice->r_it.flags = (e_bach_internal_notation_flags) (note_in_new_voice->r_it.flags & ~k_FLAG_SHIFT);
		
		if (chord_for_note_insertion) { // there's already a chord with the same onset: we add the note to the chord!
			create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)chord_for_note_insertion, k_CHORD, k_UNDO_MODIFICATION_CHANGE);

			insert_note((t_notation_obj *) x, temp_ch, note_in_new_voice, 0);
			newch = temp_ch;
			temp_ch->need_recompute_parameters = true; // we have to recalculate chord parameters 
			if (!notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)temp_ch) && !notation_item_is_preselected((t_notation_obj *) x, (t_notation_item *)temp_ch))
				notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)note_in_new_voice);
		} else { // there's no chord with the same onset. We create one.
			t_chord *newchord;
			note_in_new_voice->prev = NULL; 
			note_in_new_voice->next = NULL;
			note_in_new_voice->r_it.ID = shashtable_insert(x->r_ob.IDtable, note_in_new_voice);
			newchord = addchord_from_notes(x, note_new_voice, note->parent->onset, -1, 1, note_in_new_voice, note_in_new_voice, false, 0);

			create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)newchord, k_CHORD, k_UNDO_MODIFICATION_DELETE);

			newch = newchord;
			notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)newchord);

			// new onset?
			newch->onset += (note_new_system - note_old_system) * x->r_ob.ms_on_a_line;
			
			// assign to a group?
			if (note && newch && note->parent && note->parent->r_it.group) {
				create_header_undo_tick((t_notation_obj *)x, k_HEADER_GROUPS);
				append_element_in_group((t_notation_obj *)x, note->parent->r_it.group, (t_notation_item *)newch);
			}
		}


		// let's check if we have to transfer the j_mousedown information
		if (x->r_ob.j_mousedown_ptr == note || (x->r_ob.j_mousedown_ptr == note->parent && note->parent->num_notes == 1))
			set_mousedown((t_notation_obj *) x, note_in_new_voice, k_NOTE);

		// gotta delete the original note!!!
		t_chord *parent = note->parent;
		if (note->parent->num_notes == 1) {
			create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)note->parent, k_CHORD, k_UNDO_MODIFICATION_ADD);
			parent = NULL;
			if (old_chord_deleted)
				*old_chord_deleted = true;
		}
		note_delete((t_notation_obj *)x, note, false);

		// check lock/mute/solo compatibilities
		check_lock_mute_solo_compatibilities_for_chord_and_notes((t_notation_obj *)x, newch);
		if (parent)
			check_lock_mute_solo_compatibilities_for_chord_and_notes((t_notation_obj *)x, parent);
        
        update_all_accidentals_for_chord_if_needed(x, newch);
        if (parent)
            update_all_accidentals_for_chord_if_needed(x, parent);
		
// THIS LINE IS WRONG: if we wanted to do something like that, we should pass t_note ** note, but it's not needed 
//		note = note_in_new_voice;
	}

	verbose_print(x);	
	return newch;		
}

void clear_notes_flag_SHIFT(t_roll *x) {
	t_rollvoice *voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		t_chord *chord = voice->firstchord;
		while (chord) {
			t_note *note = chord->firstnote;
			while (note) {
				note->r_it.flags = (e_bach_internal_notation_flags) (note->r_it.flags & ~k_FLAG_SHIFT);
				note = note->next;
			}
			chord = chord->next;
		}
		voice = voice->next;
	}
}


char change_pitch_for_selection(t_roll *x, double delta, char mode, char allow_voice_change, char snap_pitch_to_grid){ 
	char changed = 0;
	t_notation_item *curr_it = x->r_ob.firstselecteditem;
	
	lock_general_mutex((t_notation_obj *)x);	
	
	clear_preselection((t_notation_obj *) x);

	// first we "mark" the selected notes. We really need this, in order to avoid confusions about the chord which will be created or merged.
	while (curr_it) { // cycle on the selected items
		if (curr_it->type == k_NOTE) { // it is a note
			t_note *nt = ((t_note *)curr_it);
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
				((t_note *)curr_it)->r_it.flags = (e_bach_internal_notation_flags) (((t_note *)curr_it)->r_it.flags | k_FLAG_SHIFT);
			}
		} else if (curr_it->type == k_CHORD) {
			t_note *curr_nt = ((t_chord *)curr_it)->firstnote;
			while (curr_nt) {
				if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt)) {
					curr_nt->r_it.flags = (e_bach_internal_notation_flags) (curr_nt->r_it.flags | k_FLAG_SHIFT);
				}
				curr_nt = curr_nt->next;
			}
		}
		curr_it = curr_it->next_selected;
	}


	curr_it = x->r_ob.firstselecteditem;
	while (curr_it) { // cycle on the selected items
		// things of which we'll have to calculate parameters: 
		t_chord *oldch = NULL; 
		t_chord *newch = NULL; 
		t_notation_item *next = curr_it->next_selected;
		char old_chord_deleted = false;
		if ((curr_it->type == k_NOTE) && (((t_note *)curr_it)->r_it.flags & k_FLAG_SHIFT)) { // it is a note
			t_note *note = (t_note *)curr_it;
			oldch = ((t_note *)curr_it)->parent;

			if (!(oldch->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
				create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)oldch, k_CHORD, k_UNDO_MODIFICATION_CHANGE);

			newch = shift_note_allow_voice_change(x, note, delta, mode, &old_chord_deleted, allow_voice_change);

			if (newch) {
				t_note *nt;
				for (nt = newch->firstnote; nt; nt = nt->next) {
					if (nt->r_it.flags & k_FLAG_SHIFT) {
						note_compute_approximation((t_notation_obj *) x, nt);
						if (change_pitch_must_actually_snap_to_grid((t_notation_obj *)x, mode, snap_pitch_to_grid)) 
							snap_pitch_to_grid_for_note((t_notation_obj *) x, nt);
					}
				}
				newch->need_recompute_parameters = true; // we have to recalculate chord parameters 
			} else {
				note_compute_approximation((t_notation_obj *) x, note);
				if (change_pitch_must_actually_snap_to_grid((t_notation_obj *)x, mode, snap_pitch_to_grid)) 
					snap_pitch_to_grid_for_note((t_notation_obj *) x, note);
			}

			if (!old_chord_deleted) {
//				t_note *nt;
//				for (nt = oldch->firstnote; nt; nt = nt->next) {
//					note_compute_approximation((t_notation_obj *) x, nt);
//					if (x->r_ob.snap_pitch_to_grid_when_editing)	
//						snap_pitch_to_grid_for_note((t_notation_obj *) x, nt);
//				}
				oldch->need_recompute_parameters = true; // we have to recalculate chord parameters 
			}
			
			changed = 1;
		} else if (curr_it->type == k_CHORD) {
			t_note *curr_nt, *temp_nt;
			oldch = ((t_chord *)curr_it);

			if (!(oldch->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
				create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)oldch, k_CHORD, k_UNDO_MODIFICATION_CHANGE);

			curr_nt = oldch->firstnote;
			while (curr_nt && !old_chord_deleted) {
				if ((curr_nt->r_it.flags & k_FLAG_SHIFT) && (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt))) {
					changed = 1;
					if (!curr_nt)
						object_warn((t_object *) x, "Error!");
					temp_nt = curr_nt;
					newch = shift_note_allow_voice_change(x, temp_nt, delta, mode, &old_chord_deleted, allow_voice_change);

					if (newch) {
						t_note *nt; 
						for (nt = newch->firstnote; nt; nt = nt->next) {
							if (true){ //(nt->flags & k_FLAG_SHIFT) {
								note_compute_approximation((t_notation_obj *) x, nt);
								if (change_pitch_must_actually_snap_to_grid((t_notation_obj *)x, mode, snap_pitch_to_grid)) 
									snap_pitch_to_grid_for_note((t_notation_obj *) x, nt);
							}
						}
						newch->need_recompute_parameters = true; // we have to recalculate chord parameters 
					}
				}
				curr_nt = curr_nt->next;
			}
			if (!old_chord_deleted) {
				t_note *nt;
				for (nt = oldch->firstnote; nt; nt = nt->next) {
					if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)) {
						note_compute_approximation((t_notation_obj *) x, nt);
						if (change_pitch_must_actually_snap_to_grid((t_notation_obj *)x, mode, snap_pitch_to_grid)) 
							snap_pitch_to_grid_for_note((t_notation_obj *) x, nt);
					}
				}
				oldch->need_recompute_parameters = true; // we have to recalculate chord parameters 
			}
		}
		curr_it = next;
	}
	
//	if (x->firstvoice->firstchord->num_notes>=2) {
//		post("diff_mc = %f", x->firstvoice->firstchord->firstnote->next->midicents - x->firstvoice->firstchord->firstnote->midicents);
//	}

	clear_notes_flag_SHIFT(x);
	move_preselecteditems_to_selection((t_notation_obj *) x, k_SELECTION_MODE_INVOLUTIVE, false, false);
	unlock_general_mutex((t_notation_obj *)x);	
	
	verbose_print(x);
	return changed;
}



// returns 1 if the point (point_x, point_y) is on the markername
int is_in_markername_shape(t_roll *x, long point_x, long point_y, t_marker *marker){
	double marker_x = onset_to_xposition((t_notation_obj *) x, marker->position_ms, NULL);
	double marker_namewidth = marker->name_uwidth * x->r_ob.zoom_y;
	double marker_name_y_start = x->r_ob.j_inset_y + 10 * x->r_ob.zoom_y + 3 * x->r_ob.zoom_y;
	double marker_nameheight = x->r_ob.markers_font_size * x->r_ob.zoom_y;
	if (marker->name_painted_direction > 0) {
		if (point_x > marker_x && point_x < marker_x + marker_namewidth + 2 * x->r_ob.zoom_y && 
			point_y > marker_name_y_start && point_y < marker_name_y_start + marker_nameheight)
			return 1;
	} else {
		if (point_x > marker_x - marker_namewidth - 2 * x->r_ob.zoom_y && point_x < marker_x && 
			point_y > marker_name_y_start && point_y < marker_name_y_start + marker_nameheight)
			return 1;
	}

	return 0;
}

void roll_okclose(t_roll *x, char *s, short *result)
{
	*result = 3;
}

void roll_edclose(t_roll *x, char **ht, long size){
	notation_obj_edclose((t_notation_obj *) x, ht, size);
}

void roll_mousedown(t_roll *x, t_object *patcherview, t_pt pt, long modifiers)
{
	double this_x, this_y;
	void* clicked_ptr = NULL; 
	e_element_types clicked_obj = k_ELEMENT_TYPES_UNDEFINED; 
	void *clicked_ptr_note = NULL;
	char clicked_slot = 0, changed = 0;
	char need_set_selection_dragging_velocity = false; 
	char need_popup = modifiers & eRightButton;
	char need_send_changed_bang = false;
		
	evnum_incr();

	llll_format_modifiers(&modifiers, NULL);
	
    x->r_ob.ux_click_marker_diff = 0;
	x->r_ob.j_mouse_is_down = false;
	x->r_ob.j_isdragging = false;
	x->r_ob.j_clicked_obj_has_been_selected = false;
    x->r_ob.check_selection_restraint_for_item = NULL;
	x->r_ob.changed_while_dragging = false;
	x->r_ob.j_mouse_hasbeendragged = 0; // we'll need this later, to check whether the mouse has been dragged
	x->r_ob.j_selection_hasbeendragged = 0; // we'll need this later, to check whether the mouse has been dragged
	x->r_ob.j_mousedown_point = pt;
	x->r_ob.j_mousedrag_point = x->r_ob.j_mousedrag_point_shift_ffk = pt; 
	// gotta find if there's an element selected! We start with notes:
	x->r_ob.j_mouse_is_down = true;
//	x->r_ob.dragging_x = pt.x;
//	x->r_ob.dragging_y = pt.y;
	x->r_ob.floatdragging_x = pt.x;
	x->r_ob.floatdragging_y = pt.y;
	x->r_ob.main_dragging_direction = 0;

    
    if (x->r_ob.notation_cursor.voice && !(!(modifiers & eAltKey) && !(modifiers & eCommandKey) && (modifiers & eControlKey))){
        roll_exit_linear_edit(x);
        return;
    }

    
	// dilation rectangle??
	if (x->r_ob.show_dilation_rectangle) {
		e_element_types clicked_point_in_dilation_rectangle = pt_to_dilation_rectangle_obj((t_notation_obj *)x, pt);
		if (!is_editable((t_notation_obj *)x, k_DILATION_RECTANGLE, k_ELEMENT_ACTIONS_NONE)) return;
		if (clicked_point_in_dilation_rectangle > 0) {
			clicked_ptr = WHITENULL;
			clicked_obj = clicked_point_in_dilation_rectangle;
			set_mousedown((t_notation_obj *) x, clicked_ptr, clicked_obj);
			return;
		} else {
			x->r_ob.show_dilation_rectangle = false;
			bach_set_cursor((t_object *)x, &x->r_ob.j_mouse_cursor, patcherview, BACH_CURSOR_DEFAULT);
		}
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	}
	
	this_x = pt.x; this_y = pt.y; // click-point coordinates

	
	
	
	// popup menu?
	if (need_popup) {
		t_rollvoice *voice;
		if (!is_editable((t_notation_obj *)x, k_POPUP_MENU, k_ELEMENT_ACTIONS_NONE)) return;
		for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
			
			if (((t_voice *)voice)->hidden || ((t_voice *)voice)->part_index != 0) continue;
			
			if (is_in_clef_shape((t_notation_obj *) x, this_x, this_y, (t_voice *) voice)) {
				int chosenelem = 0, chosenclef = k_CLEF_WRONG; 
				t_symbol *chosenkeysym;

				// tries to find if one of the common commands has been selected, if not we use <chosenelem>
				char res = handle_voice_popup((t_notation_obj *)x, (t_voice *)voice, modifiers, &chosenelem);
				
				if (res != k_CHANGED_DO_NOTHING){ 
					handle_change((t_notation_obj *)x, res, k_UNDO_OP_UNKNOWN);
				} else {
					// insert voice?
					if (chosenelem == 2001 || chosenelem == 2002) {
						if (!is_editable((t_notation_obj *)x, k_VOICE, k_CREATION)) return;
						create_whole_roll_undo_tick(x);
                        roll_move_and_reinitialize_last_voice(x, chosenelem == 2002 ? (t_rollvoice *)voiceensemble_get_lastvoice((t_notation_obj *)x, (t_voice *)voice) : (voiceensemble_get_firstvoice((t_notation_obj *)x, (t_voice *)voice) ? ((t_rollvoice *)voiceensemble_get_firstvoice((t_notation_obj *)x, (t_voice *)voice))->prev : voice->prev), x->r_ob.keys_as_symlist[voice->v_ob.number], get_voice_clef((t_notation_obj *)x, (t_voice *)voice), llll_get(), voice->v_ob.midichannel, voice->v_ob.number + 1);
						handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_INSERT_VOICE);
						return;
					}
					
					// move voice
					if (chosenelem == 2003 || chosenelem == 2004) {
						if (!is_editable((t_notation_obj *)x, k_VOICE, k_MODIFICATION_GENERIC)) return;
						create_whole_roll_undo_tick(x);
						roll_swap_voiceensembles(x, voice, chosenelem == 2003 ? voice->prev : voice->next);
						handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_MOVE_VOICE);
						return;
					}
					
					// delete voice?
					if (chosenelem == 2000) {
						if (!is_editable((t_notation_obj *)x, k_VOICE, k_DELETION)) return;
						create_whole_roll_undo_tick(x);
						roll_delete_voiceensemble(x, (t_voice *)voice);
						handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_VOICE);
						return;
					}
					
					// clefs ?
					chosenclef = popup_menu_result_to_clef((t_notation_obj *) x, chosenelem);
					if (chosenclef != k_CLEF_WRONG) {
						if (!is_editable((t_notation_obj *)x, k_VOICE, k_MODIFICATION_CLEF)) return;
						change_voiceensemble_clef((t_notation_obj *) x, (t_voice *)voice, chosenclef, true);
						handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_CLEFS);
					} 
					
					// keys?
					chosenkeysym = popup_menu_result_to_keysymbol((t_notation_obj *) x, chosenelem);
					if (chosenkeysym) {
						t_atom av[CONST_MAX_VOICES];
						long i; 
						t_rollvoice *tmpvoice;
						
						if (!is_editable((t_notation_obj *)x, k_VOICE, k_MODIFICATION_KEY)) return;

						create_header_undo_tick((t_notation_obj *)x, k_HEADER_KEYS);
						
						for (i = 0, tmpvoice = x->firstvoice; i < x->r_ob.num_voices && tmpvoice; i++, tmpvoice = tmpvoice->next) {
							if (do_voices_belong_to_same_voiceensemble((t_notation_obj *)x, (t_voice *)tmpvoice, (t_voice *)voice))
								atom_setsym(av+i, chosenkeysym);
							else
								atom_setsym(av+i, x->r_ob.keys_as_symlist[i]);
						}
						roll_setattr_keys(x, NULL, x->r_ob.num_voices, av);
						
						handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_KEYS);
					}

					// midichannels?
					if (chosenelem > 150 && chosenelem <= 166){
						if (!is_editable((t_notation_obj *)x, k_VOICE, k_MODIFICATION_GENERIC)) return;
						change_voiceensemble_midichannel((t_notation_obj *)x, (t_voice *)voice, chosenelem - 150, true);
						handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_MIDICHANNELS);
					}
					
				}
				
				return; 
			}
		}
	}
	
	lock_general_mutex((t_notation_obj *)x);	
	
	clear_preselection((t_notation_obj *) x); // we clear the previous pre-selection. 
	// not necessarily do we clear the selection as well: think at the case when a selection is already done, 
	// and the user click on a note of that selection, in order to drag it! 
	// So we handle that later.
	
	// first of all: are we in a slot mode???? Cause if we are in a slot mode, we gotta handle that separately
	if (x->r_ob.active_slot_num > -1 && !is_editable((t_notation_obj *)x, k_SLOT, k_ELEMENT_ACTIONS_NONE)) {
		unlock_general_mutex((t_notation_obj *)x);	
		return;
	}

	clicked_slot = handle_slot_mousedown((t_notation_obj *) x, patcherview, pt, modifiers, &clicked_obj, &clicked_ptr, &changed, need_popup);
	
	if (clicked_slot) {
		unlock_general_mutex((t_notation_obj *)x);	
		handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_SEND_BANG, k_UNDO_OP_CHANGE_SLOT);
		return;
	}
	
	
	if (modifiers == eControlKey + eAltKey) {
		clicked_ptr = WHITENULL;
		clicked_obj = k_DRAG;
		if (is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE))
			set_mousedown((t_notation_obj *) x, clicked_ptr, clicked_obj);
		
		unlock_general_mutex((t_notation_obj *)x);	
		return;
	} else if (modifiers == eCommandKey + eAltKey || modifiers == eCommandKey + eAltKey + eShiftKey) {
		clicked_obj = k_ZOOMING_REGION;
		clicked_ptr = WHITENULL;
		if (is_editable((t_notation_obj *)x, k_ZOOMING_REGION, k_ELEMENT_ACTIONS_NONE))
			set_mousedown((t_notation_obj *) x, clicked_ptr, clicked_obj);
		unlock_general_mutex((t_notation_obj *)x);	
		return;
	}

	// clicked hscrollbar?
	if (!clicked_ptr && x->r_ob.need_hscrollbar && x->r_ob.show_hscrollbar) {	
		char res;
		
		res = is_in_scrollbar_shape((t_notation_obj *) x, this_x, this_y);

		if (res != 0 && !is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE)) {
			unlock_general_mutex((t_notation_obj *)x);	
			return;
		}

		if (res == 1) {
			clicked_ptr = WHITENULL; //doesn't really matter, but NON 0...
			clicked_obj = k_SCROLLBAR;
		} else if (res == -1) {
			clicked_ptr = WHITENULL;
			clicked_obj = k_SCROLLBAR_LEFT_AREA;
			x->r_ob.hscrollbar_x = x->r_ob.hscrollbar_x - x->r_ob.hscrollbar_width;
			update_hscrollbar((t_notation_obj *) x, 0);
			unlock_general_mutex((t_notation_obj *)x);	
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
			set_mousedown((t_notation_obj *) x, clicked_ptr, clicked_obj);
			send_domain(x, 6, NULL);
			return;
		} else if (res == -2) {
			clicked_ptr = WHITENULL;
			clicked_obj = k_SCROLLBAR_RIGHT_AREA;
			x->r_ob.hscrollbar_x = x->r_ob.hscrollbar_x + x->r_ob.hscrollbar_width;
			update_hscrollbar((t_notation_obj *) x, 0);
			unlock_general_mutex((t_notation_obj *)x);	
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
			send_domain(x, 6, NULL);
			set_mousedown((t_notation_obj *) x, clicked_ptr, clicked_obj);
			return;
		}
	}
	
	
	// clicked vscrollbar?
	if (!clicked_ptr && x->r_ob.need_vscrollbar && x->r_ob.show_vscrollbar) {	
		char res;
		
		res = is_in_vscrollbar_shape((t_notation_obj *) x, this_x, this_y);

        if (res != 0 && !is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE)) {
            unlock_general_mutex((t_notation_obj *)x);
            return;
        }
        
        if (res == 1) {
			clicked_ptr = WHITENULL; //doesn't really matter, but NON 0...
			clicked_obj = k_VSCROLLBAR;
		} else if (res == -1) {
			clicked_ptr = WHITENULL;
			clicked_obj = k_SCROLLBAR_TOP_AREA;
			x->r_ob.vscrollbar_y -= x->r_ob.vscrollbar_height;
			update_vscrollbar((t_notation_obj *) x, 0);
			unlock_general_mutex((t_notation_obj *)x);	
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
			set_mousedown((t_notation_obj *) x, clicked_ptr, clicked_obj);
			//			send_vdomain(x, 6, NULL);
			return;
		} else if (res == -2) {
			clicked_ptr = WHITENULL;
			clicked_obj = k_SCROLLBAR_BOTTOM_AREA;
			x->r_ob.vscrollbar_y += x->r_ob.vscrollbar_height;
			update_vscrollbar((t_notation_obj *) x, 0);
			unlock_general_mutex((t_notation_obj *)x);	
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
			//			send_vdomain(x, 6, NULL);
			set_mousedown((t_notation_obj *) x, clicked_ptr, clicked_obj);
			return;
		}
	}
	
	// changing pos of a loop extreme?
	if (!clicked_ptr && x->r_ob.show_loop_region && !(modifiers & eAltKey) && !(modifiers & eCommandKey) && !(modifiers & eControlKey)) {
		t_rect rect;
		jbox_get_rect_for_view(&x->r_ob.j_box.l_box.b_ob, patcherview, &rect);
		e_element_types pos = is_in_loop_region((t_notation_obj *)x, rect, this_x, this_y);
		if (pos != k_NONE) {
			unlock_general_mutex((t_notation_obj *)x);	
            notationobj_redraw((t_notation_obj *) x);
			if (is_editable((t_notation_obj *)x, k_LOOP_REGION, k_ELEMENT_ACTIONS_NONE))
				send_loop_region((t_notation_obj *) x, 6);
			set_mousedown((t_notation_obj *) x, WHITENULL_llll, pos);
			return;
		}
	}
	
    
    // clicked clef?
    if (!clicked_ptr) {
        long voicenum = yposition_to_voicenumber((t_notation_obj *)x, this_y, 0, k_VOICEENSEMBLE_INTERFACE_FIRST);
        t_rollvoice *voice = nth_rollvoice(x, voicenum);
        if (is_in_clef_shape((t_notation_obj *)x, pt.x, pt.y, (t_voice *)voice)) {
            clicked_ptr = voice;
            clicked_obj = k_VOICE;
            if (is_editable((t_notation_obj *)x, k_SELECTION, k_SINGLE_SELECTION) || is_editable((t_notation_obj *)x, k_SELECTION, k_MULTIPLE_SELECTION))
                delete_item_type_from_selection((t_notation_obj *) x, -k_VOICE); // we only keep voices in selection
        }
    }
    
    
	// clicked note? 
	if (!clicked_ptr) { // looking for the clicked note, if any
		t_rollvoice *voice;
		char clicked_is_selected = false;
		double clicked_onset = 0;
		for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
			t_chord *curr_ch;
			for (curr_ch = voice->firstchord; curr_ch; curr_ch = curr_ch->next){
				t_note *curr_nt;
				
				if (clicked_ptr && (clicked_is_selected || fabs(curr_ch->onset - clicked_onset) * CONST_X_SCALING * x->r_ob.zoom_x > x->r_ob.noteheads_typo_preferences.nhpref[k_NOTEHEAD_DOUBLE_WHOLE_NOTE].uwidth))
					break;

				for (curr_nt = curr_ch->firstnote; curr_nt; curr_nt = curr_nt->next){
					char is_note_generally_selected = notation_item_is_globally_selected((t_notation_obj *) x, (t_notation_item *)curr_nt);

					if (clicked_ptr && (clicked_is_selected || fabs(curr_ch->onset - clicked_onset) > CONST_X_SCALING * x->r_ob.zoom_x * x->r_ob.zoom_x))
						break;
					
					if ((!clicked_ptr || is_note_generally_selected) && is_in_note_shape((t_notation_obj *)x,curr_nt,this_x,this_y)) {
						
						if (need_popup){
							long res = k_CHANGED_DO_NOTHING;
							unlock_general_mutex((t_notation_obj *)x);	

							if (!is_editable((t_notation_obj *)x, k_POPUP_MENU, k_ELEMENT_ACTIONS_NONE)) return;
							res = handle_note_popup((t_notation_obj *) x, curr_nt, modifiers, clipboard.type);
                            if (res == 501) {
                                // add marker
                                lock_general_mutex((t_notation_obj *)x);
                                lock_markers_mutex((t_notation_obj *)x);
                                if (is_editable((t_notation_obj *)x, k_MARKER, k_CREATION)) {
                                    t_llll *names = find_unused_marker_names((t_notation_obj *) x, NULL, NULL);
                                    double onset = curr_nt->parent->onset;
                                    create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
                                    clicked_ptr = add_marker((t_notation_obj *) x, names, onset, build_timepoint(0, long2rat(0)), k_MARKER_ATTACH_TO_MS, k_MARKER_ROLE_NONE, NULL, 0);
                                    clicked_obj = k_MARKER;
                                    if (x->r_ob.snap_markers_to_grid_when_editing)
                                        snap_onset_to_grid_for_marker((t_notation_obj *) x, (t_marker *)clicked_ptr, NULL);
                                    recompute_total_length((t_notation_obj *)x);
                                    x->r_ob.item_changed_at_mousedown = 1;
                                    llll_free(names);
                                }
                                unlock_markers_mutex((t_notation_obj *)x);
                                unlock_general_mutex((t_notation_obj *)x);
                                handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_ADD_MARKER);
                                return;
                            } else if (res == 473 && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET)) {
								align_selection_onsets(x);
								handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_ALIGN_SELECTION);
							} else if (res == 474 && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET))  {
								equally_respace_selection_onsets(x);
								handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_EQUALLY_RESPACE_SELECTION);
							} else if (res == 475 && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_DURATION)) {
								legato(x, 0);
								handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LEGATO_FOR_SELECTION);
                            } else if (res == 950) { // delete selection
                                roll_delete_selection_and_transfer_default_slots(x, false);
                                handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_SELECTION);
                            } else if (res == 951) { // ripple delete selection
                                roll_delete_selection_and_transfer_default_slots(x, true);
                                handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_RIPPLE_DELETE_SELECTION);
                            } else if (res == 970) { // copy selection
                                roll_copy_selection(x, false);
                            } else if (res == 971) { // copy duration line
                                notation_obj_copy_durationline((t_notation_obj *)x, &clipboard, curr_nt, false);
                            } else if (res >= 9000 && res <= 9000 + CONST_MAX_SLOTS + 1) { // copy slot
                                notation_obj_copy_slot((t_notation_obj *)x, &clipboard, (t_notation_item *)curr_nt, res - 9000 - 1, false);
                            } else if (res == 980) { // paste replace
                                if (clipboard.type == k_SELECTION_CONTENT) {
                                    double onset = get_selection_leftmost_onset(x);
                                    long voice = get_selection_topmost_voice(x);
                                    roll_delete_selection(x, false);
                                    if (voice >= 0)
                                        roll_paste_clipboard(x, false, onset, false, voice, true, true);
                                }
                            } else if (res == 981) { // paste duration line
                                if (clipboard.type == k_DURATION_LINE)
                                    notation_obj_paste_durationline((t_notation_obj *)x, &clipboard);
                            } else if (res >= 10000 && res <= 10000 + CONST_MAX_SLOTS + 1) { // paste slot
                                if (clipboard.type == k_SLOT)
                                    notation_obj_paste_slot((t_notation_obj *) x, &clipboard, res - 10000 - 1);
							} else if (res != k_CHANGED_DO_NOTHING)
								handle_change((t_notation_obj *)x, res, k_UNDO_OP_UNKNOWN);
							return;
						} else if ((modifiers == eCommandKey) && !notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt)) { // delete note!
							if (is_editable((t_notation_obj *)x, curr_nt->parent->num_notes == 1 ? k_CHORD : k_NOTE, k_DELETION)) {
                                t_rollvoice *voice = curr_nt->parent->voiceparent;
								clicked_ptr = NULL;
								create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)curr_nt->parent, curr_nt->parent->num_notes == 1 ? k_UNDO_MODIFICATION_ADD : k_UNDO_MODIFICATION_CHANGE);
								note_delete((t_notation_obj *)x, curr_nt, true);
                                update_all_accidentals_for_voice_if_needed(x, voice);
								x->r_ob.item_changed_at_mousedown = 1;
							}
							unlock_general_mutex((t_notation_obj *)x);	
							handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_NOTE);
							return;
						} else if ((t_note *)clicked_ptr != curr_nt) {
							clicked_ptr = curr_nt;
							clicked_obj = k_NOTE;
							
							clicked_is_selected = is_note_generally_selected;
							clicked_onset = curr_ch->onset;
							need_set_selection_dragging_velocity = true; 
							break;
						}
					}
				} 

				// lyrics
				if (!clicked_ptr && x->r_ob.link_lyrics_to_slot > 0 && x->r_ob.show_lyrics) {
					if (is_in_chord_lyrics_shape((t_notation_obj *) x, curr_ch, this_x, this_y)){
						clicked_ptr = curr_ch->lyrics;
						clicked_obj = k_LYRICS;
						break;
					}
				}

                // dynamics
                if (!clicked_ptr && x->r_ob.link_dynamics_to_slot > 0 && x->r_ob.show_dynamics) {
                    if (is_in_chord_dynamics_shape((t_notation_obj *) x, curr_ch, this_x, this_y)){
                        clicked_ptr = curr_ch->dynamics;
                        clicked_obj = k_DYNAMICS;
                        break;
                    }
                }
                // articulations
                if (!clicked_ptr && x->r_ob.show_articulations && x->r_ob.link_articulations_to_slot > 0 && x->r_ob.link_articulations_to_slot <= CONST_MAX_SLOTS) {
                    long s = x->r_ob.link_articulations_to_slot - 1;
                    if (x->r_ob.slotinfo[s].slot_type == k_SLOT_TYPE_ARTICULATIONS) {
                        t_slotitem *item;
                        t_note *curr_nt;
                        for (curr_nt = curr_ch->firstnote; curr_nt && !clicked_ptr; curr_nt = curr_nt->next) {
                            for (item = curr_nt->slot[s].firstitem; item; item = item->next){
                                t_articulation *art = (t_articulation *)item->item;
                                if (is_in_articulation_shape((t_notation_obj *)x, art, this_x, this_y)){
                                    if (need_popup) {
                                        long res = k_CHANGED_DO_NOTHING;
                                        unlock_general_mutex((t_notation_obj *)x);
                                        if (is_editable((t_notation_obj *)x, k_ARTICULATION, k_MODIFICATION_GENERIC)) {
                                            res = handle_articulations_popup((t_notation_obj *) x, art, modifiers);
                                            handle_change((t_notation_obj *)x, res, k_UNDO_OP_UNKNOWN);
                                        }
                                        return;
                                    } else if (modifiers == eCommandKey && (!notation_item_is_globally_locked((t_notation_obj *) x, (t_notation_item *)curr_ch))) { // delete articulation
                                        if (is_editable((t_notation_obj *)x, k_ARTICULATION, k_DELETION)) {
                                            create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)curr_ch, k_UNDO_MODIFICATION_CHANGE);
                                            delete_slotitem((t_notation_obj *)x, s, item);
                                            handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_ARTICULTATION);
                                            x->r_ob.item_changed_at_mousedown = 1;
                                        }
                                        unlock_general_mutex((t_notation_obj *)x);
                                        return;
                                    } else {
                                        clicked_ptr = art;
                                        clicked_obj = k_ARTICULATION;
                                    }
                                }
                            }
                        }
                    }
                }
            }
		}
	}

	
	// clicked tail?
	if (!clicked_ptr && (x->r_ob.show_durations) && (modifiers != eCommandKey)) {
		t_rollvoice *voice = x->firstvoice;
		t_chord *curr_ch;
		while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
			curr_ch = voice->firstchord;
			while ((curr_ch)&&(!(clicked_ptr))) { // cycle on the chords
				//			post("chord onset: %ld, chord numnotes: %d", curr_ch->onset, curr_ch->num_notes);
				t_note *curr_nt = curr_ch->firstnote;
				//			post("note midic: %ld, duration: %ld, velocity: %ld, next: %lx", curr_nt->midicents, curr_nt->duration, curr_nt->velocity, curr_nt->next);
				while ((curr_nt)&&(!(clicked_ptr))) { // cycle on the notes
					if (is_in_tail_shape((t_notation_obj *)x,curr_nt,this_x,this_y)) {
						clicked_ptr = curr_nt->lastbreakpoint;
						clicked_obj = k_PITCH_BREAKPOINT;
						need_set_selection_dragging_velocity = true; 
						break;
					}
					curr_nt = curr_nt->next;
				} 
				curr_ch = curr_ch->next;
			}
			voice = voice->next;
		}
	}
	
	
	// clicked breakpoint?
	if (!clicked_ptr && x->r_ob.show_durations && x->r_ob.allow_glissandi) { // looking for the clicked durationline, if any
		t_rollvoice *voice = x->firstvoice;
		t_chord *curr_ch;
		while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
			curr_ch = voice->firstchord;
			while (curr_ch && !clicked_ptr) { // cycle on the chords
				t_note *curr_nt = curr_ch->firstnote;
				while (curr_nt && !clicked_ptr) { // cycle on the notes
					if (curr_nt->num_breakpoints > 2) { // if there are nontrivial breakpoints
						t_bpt *curr_bpt = curr_nt->firstbreakpoint->next;
						while (curr_bpt && !clicked_ptr) { // cycle on the breakpoints
							if (is_in_breakpoint_shape((t_notation_obj *)x, curr_bpt, this_x, this_y)) {
								if ((modifiers == eCommandKey) && !notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt)) { // delete bpt!
									if (is_editable((t_notation_obj *)x, k_PITCH_BREAKPOINT, k_DELETION)) {
										clicked_ptr = NULL;
										create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)curr_ch, k_UNDO_MODIFICATION_CHANGE);
										delete_breakpoint((t_notation_obj *) x, curr_bpt);
										x->r_ob.item_changed_at_mousedown = 1;
									}
									unlock_general_mutex((t_notation_obj *)x);
									handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_PITCH_BREAKPOINT);
									return;
								} else {
									clicked_ptr = curr_bpt;
									clicked_obj = k_PITCH_BREAKPOINT;
									need_set_selection_dragging_velocity = true; 
									break;
								}
							}
							curr_bpt = curr_bpt->next;
						}
					}
					curr_nt = curr_nt->next;
				} 
				curr_ch = curr_ch->next;
			}
			voice = voice->next;
		}
	}
	
	// clicked durationline?
	if (!clicked_ptr && x->r_ob.show_durations && modifiers != eCommandKey) { // looking for the clicked durationline, if any
		t_rollvoice *voice = x->firstvoice;
		t_chord *curr_ch;
		while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
			curr_ch = voice->firstchord;
			while ((curr_ch)&&(!(clicked_ptr))) { // cycle on the chords
				t_note *curr_nt = curr_ch->firstnote;
				while (curr_nt && !clicked_ptr) { // cycle on the notes
					if (is_in_durationline_shape((t_notation_obj *)x,curr_nt,this_x,this_y)) {
                        
                        
                        if (need_popup) {
                            // popup menu
                            long res = k_CHANGED_DO_NOTHING;
                            unlock_general_mutex((t_notation_obj *)x);
                            
                            if (!is_editable((t_notation_obj *)x, k_POPUP_MENU, k_ELEMENT_ACTIONS_NONE)) return;
                            res = handle_durationline_popup((t_notation_obj *) x, curr_nt->durationline, modifiers);
                            if (res == 8000 && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_GENERIC) && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_CREATION)) {
                                roll_slice_note(x, curr_nt, xposition_to_onset((t_notation_obj *) x, this_x, 0));
                                handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_SLICE);
                            } else if (res != k_CHANGED_DO_NOTHING)
                                handle_change((t_notation_obj *)x, res, k_UNDO_OP_UNKNOWN);
                            return;
                        } else if ((modifiers == eAltKey) && (x->r_ob.allow_glissandi) && !notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt)) {
							// add a breakpoint!
							if (is_editable((t_notation_obj *)x, k_PITCH_BREAKPOINT, k_CREATION)) {
								double start_x = onset_to_xposition((t_notation_obj *) x, curr_ch->onset, NULL);
								double end_x = onset_to_xposition((t_notation_obj *) x, curr_ch->onset + curr_nt->duration, NULL);
								double rel_x_pos = (end_x > start_x) ? (this_x-start_x)/(end_x-start_x) : -1.;
								double y_pos = 0; //yposition_to_mc(x, this_y) - curr_nt->midicents;
								t_bpt *this_bpt;
								
								create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)curr_ch, k_UNDO_MODIFICATION_CHANGE);
								this_bpt = add_breakpoint((t_notation_obj *) x, curr_nt, rel_x_pos, y_pos, 0., true, CONST_DEFAULT_NEW_NOTE_VELOCITY, true); 
								x->r_ob.item_changed_at_mousedown = 1;
								//							x->r_ob.changed_while_dragging = true;
								need_send_changed_bang = true;
								clicked_ptr = this_bpt;
								clicked_obj = k_PITCH_BREAKPOINT;
							}
//							unlock_general_mutex((t_notation_obj *)x);	
//							handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_ADD_PITCH_BREAKPOINT);
//							lock_general_mutex((t_notation_obj *)x);	
							break;
                        } else if (modifiers == eShiftKey + eControlKey && !notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt)) {
                            // cut note
                            clicked_ptr = roll_slice_note(x, curr_nt, xposition_to_onset((t_notation_obj *) x, this_x, 0));
                            clicked_obj = k_NOTE;
                            break;
                        } else {
							clicked_ptr = curr_nt->durationline;
							clicked_obj = k_DURATION_LINE; // was: k_NOTE
							need_set_selection_dragging_velocity = true; 
							break;
						}
					}
					curr_nt = curr_nt->next;
				} 
				curr_ch = curr_ch->next;
			}
			voice = voice->next;
		}
	}


	
	// clicked marker?
	lock_markers_mutex((t_notation_obj *)x);;
	if (!clicked_ptr && x->r_ob.show_markers && x->r_ob.firstmarker) {
		t_marker *marker;
		for (marker = x->r_ob.firstmarker; marker; marker = marker->next) {
			if (is_in_marker_shape((t_notation_obj *)x, marker, this_x, this_y) || is_in_markername_shape((t_notation_obj *)x, marker, this_x, this_y)){
				if (modifiers == eCommandKey) {
					if (is_editable((t_notation_obj *)x, k_MARKER, k_DELETION)) {
						create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
						delete_marker((t_notation_obj *) x, marker); 
						x->r_ob.item_changed_at_mousedown = 1;
						changed = true;
						recompute_total_length((t_notation_obj *)x);
						update_hscrollbar((t_notation_obj *)x, 0);
					}
//					x->r_ob.changed_while_dragging = true;
					unlock_markers_mutex((t_notation_obj *)x);;	
					unlock_general_mutex((t_notation_obj *)x);	
					handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_MARKER);
					return;
				} else {
                    x->r_ob.ux_click_marker_diff = xposition_to_unscaled_xposition((t_notation_obj *)x, this_x) - onset_to_unscaled_xposition((t_notation_obj *)x, marker->position_ms);
                    
                    t_marker *temp;
                    double marker_mousedown_ux = get_marker_ux_position((t_notation_obj *)x, marker);
                    for (temp = x->r_ob.firstmarker; temp; temp = temp->next){
                        double this_ux = get_marker_ux_position((t_notation_obj *)x, temp);
                        temp->ux_difference_with_mousedown_marker = this_ux - marker_mousedown_ux;
                    }

                    clicked_ptr = marker;
					clicked_obj = k_MARKER;
					break;
				}
			}
		} 
	}

    // changing pos of playcursor?
    if (!clicked_ptr && x->r_ob.show_playhead && modifiers == eAltKey) {
        clicked_ptr = WHITENULL; //doesn't really matter, but NON 0...
        clicked_obj = k_PLAYCURSOR;
        if (x->r_ob.playhead_dragging_mode != 1 && is_editable((t_notation_obj *)x, k_PLAYCURSOR, k_ELEMENT_ACTIONS_NONE))
            x->r_ob.play_head_start_ms = xposition_to_onset((t_notation_obj *) x, x->r_ob.j_mousedown_point.x, yposition_to_systemnumber((t_notation_obj *) x, x->r_ob.j_mousedown_point.y));
        unlock_general_mutex((t_notation_obj *)x);
        unlock_markers_mutex((t_notation_obj *)x);
        notationobj_redraw((t_notation_obj *) x);
        if (x->r_ob.playhead_dragging_mode != 1 && is_editable((t_notation_obj *)x, k_PLAYCURSOR, k_ELEMENT_ACTIONS_NONE)) {
            send_moved_playhead_position((t_notation_obj *) x, 6);
        }
        set_mousedown((t_notation_obj *) x, clicked_ptr, clicked_obj);
        return;
    }
    
	// adding a marker?
	if (!clicked_ptr && x->r_ob.show_markers && modifiers == eAltKey + eShiftKey) {
		double onset = xposition_to_onset((t_notation_obj *) x, x->r_ob.j_mousedown_point.x, yposition_to_systemnumber((t_notation_obj *) x, x->r_ob.j_mousedown_point.y));
		if (onset >= 0) { 
			if (is_editable((t_notation_obj *)x, k_MARKER, k_CREATION)) {
				t_llll *names = find_unused_marker_names((t_notation_obj *) x, NULL, NULL);
				create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
				clicked_ptr = add_marker((t_notation_obj *) x, names, onset, build_timepoint(0, long2rat(0)), k_MARKER_ATTACH_TO_MS, k_MARKER_ROLE_NONE, NULL, 0);
				clicked_obj = k_MARKER;
                if (x->r_ob.snap_markers_to_grid_when_editing)
                    snap_onset_to_grid_for_marker((t_notation_obj *) x, (t_marker *)clicked_ptr, NULL);
                recompute_total_length((t_notation_obj *)x);
				x->r_ob.item_changed_at_mousedown = 1;
				llll_free(names);
			}
//			x->r_ob.changed_while_dragging = true;
			unlock_markers_mutex((t_notation_obj *)x);;	
			unlock_general_mutex((t_notation_obj *)x);	
			handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_ADD_MARKER);
			return;
		}
	}
	unlock_markers_mutex((t_notation_obj *)x);;	

	
    // start editing in linear editing system?
    if (!clicked_ptr && !(modifiers & eAltKey) && !(modifiers & eCommandKey) && (modifiers & eControlKey) && x->r_ob.allow_linear_edit){
        long voicenum = yposition_to_voicenumber((t_notation_obj *)x, this_y, -1, k_VOICEENSEMBLE_INTERFACE_FIRST);
        t_rollvoice *voice = nth_rollvoice(x, voicenum);
        double mc = yposition_to_mc((t_notation_obj *)x, this_y, (t_voice *)voice, NULL);

        if (is_y_within_voice_staff((t_notation_obj *) x, this_y, (t_voice *)voice)) {
            long screen_nt;
            t_rational screen_acc;
            x->r_ob.notation_cursor.onset = xposition_to_ms((t_notation_obj *)x, x->r_ob.j_mousedown_point.x, 0);
            x->r_ob.notation_cursor.measure = NULL;
            x->r_ob.notation_cursor.voice = (t_voice *)voice;
            x->r_ob.notation_cursor.midicents = round(mc/100) * 100;
            mc_to_screen_approximations((t_notation_obj *) x, x->r_ob.notation_cursor.midicents, &screen_nt, &screen_acc, voice->v_ob.acc_pattern, voice->v_ob.full_repr);
            x->r_ob.notation_cursor.midicents = screen_nt;
            x->r_ob.notation_cursor.step = midicents_to_diatsteps_from_middleC((t_notation_obj *) x, screen_nt);
            if (x->r_ob.show_grid && x->r_ob.snap_linear_edit_to_grid_when_editing)
                roll_linear_edit_snap_cursor_to_grid(x); // snap to chord is then inside
            else
                roll_linear_edit_snap_to_chord(x); // just to snap to chord
            move_linear_edit_cursor_depending_on_edit_ranges((t_notation_obj *)x, 0, 0);
            unlock_general_mutex((t_notation_obj *)x);
            return;
        }
    }

    
    
	// setting loop?
	if (!clicked_ptr && x->r_ob.show_loop_region && modifiers & eControlKey && !(modifiers & eAltKey) && !(modifiers & eCommandKey)) {
		double this_ms = xposition_to_onset((t_notation_obj *)x, x->r_ob.j_mousedown_point.x, yposition_to_systemnumber((t_notation_obj *) x, x->r_ob.j_mousedown_point.y));
		if (modifiers & eShiftKey) {
			t_chord *nearest_chord = get_nearest_chord(x, this_ms);
			if (nearest_chord)
				this_ms = nearest_chord->onset;
		}
		unlock_general_mutex((t_notation_obj *)x);	
		set_loop_region_from_extremes(x, this_ms, this_ms);
        notationobj_redraw((t_notation_obj *) x);
		if (is_editable((t_notation_obj *)x, k_LOOP_REGION, k_ELEMENT_ACTIONS_NONE))
			send_loop_region((t_notation_obj *) x, 6);
		set_mousedown((t_notation_obj *) x, WHITENULL_llll, k_LOOP_END);
		return;
	}
	
	if (!clicked_ptr && need_popup) {
		long res;
		unlock_general_mutex((t_notation_obj *)x);
		if (!is_editable((t_notation_obj *)x, k_POPUP_MENU, k_ELEMENT_ACTIONS_NONE)) return;
		res = handle_background_popup((t_notation_obj *)x, modifiers, clipboard.type);
        if (res == 501) {
            // add marker
            lock_general_mutex((t_notation_obj *)x);
            lock_markers_mutex((t_notation_obj *)x);
            if (is_editable((t_notation_obj *)x, k_MARKER, k_CREATION)) {
                t_llll *names = find_unused_marker_names((t_notation_obj *) x, NULL, NULL);
                double onset = xposition_to_onset((t_notation_obj *) x,pt.x, yposition_to_systemnumber((t_notation_obj *) x, pt.y));
                create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
                clicked_ptr = add_marker((t_notation_obj *) x, names, onset, build_timepoint(0, long2rat(0)), k_MARKER_ATTACH_TO_MS, k_MARKER_ROLE_NONE, NULL, 0);
                clicked_obj = k_MARKER;
                if (x->r_ob.snap_markers_to_grid_when_editing)
                    snap_onset_to_grid_for_marker((t_notation_obj *) x, (t_marker *)clicked_ptr, NULL);
                recompute_total_length((t_notation_obj *)x);
                x->r_ob.item_changed_at_mousedown = 1;
                llll_free(names);
            }
            unlock_markers_mutex((t_notation_obj *)x);
            unlock_general_mutex((t_notation_obj *)x);
            handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_ADD_MARKER);
            return;
        } else if (res == 990) { // paste selection
            double onset = xposition_to_onset((t_notation_obj *) x, x->r_ob.j_mouse_x, 0);
            long voice = yposition_to_voicenumber((t_notation_obj *)x, x->r_ob.j_mouse_y, -1, k_VOICEENSEMBLE_INTERFACE_ACTIVE);
            if (voice >= 0)
                roll_paste_clipboard(x, false, onset, false, voice, true, true);
        } else if (res == 991) { // paste at original location
             roll_paste_clipboard(x, true, 0, true, 0, true, true);
        } else
            handle_change((t_notation_obj *)x, res, k_UNDO_OP_UNKNOWN);
		return;
	}
	
	//		post("clicked_NN: %d, clicked obj: %d", clicked_NN, clicked_obj);
	
	// redraw if needed
	if (clicked_ptr)	
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	
	if (!clicked_ptr && clicked_obj != k_SCROLLBAR && clicked_obj != k_VSCROLLBAR) {
		// we start a region determination (the user is either just clicking, or (pre)selecting a region.
		clicked_obj = k_REGION;
	}
	
	if (clicked_ptr && clicked_obj == k_NOTE)
		clicked_ptr_note = clicked_ptr;
	
	e_element_actions selection_action = (x->r_ob.num_selecteditems == 0) ? k_SINGLE_SELECTION : k_MULTIPLE_SELECTION;
	
	// see if we've clicked on a note which itself constitues a chord. In this case, change to chord selection.
	if (clicked_obj == k_NOTE &&
		( !(((t_note *)clicked_ptr)->r_it.selected || ((t_note *)clicked_ptr)->parent->r_it.selected) || // the note is NOT selected...
		  (((t_note *)clicked_ptr)->parent->r_it.selected && ((t_note *)clicked_ptr)->parent->num_notes == 1)) // ...or: the note is selected, but as the single-note whole chord
		) { 
		
		// let's see if we're selecting a note of a chord with just 1 note. In that case, the whole chord is selected.
		long count_notes_in_region = 0;
		char is_chord_selected = notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)((t_note *)clicked_ptr)->parent);
		t_note *curr_nt = ((t_note *)clicked_ptr)->parent->firstnote;
		while (curr_nt) { // how many notes are inside the selection?
			if ((curr_nt != clicked_ptr) && (is_chord_selected || ((notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)curr_nt)) && (modifiers & eShiftKey))))
				count_notes_in_region++;
			curr_nt = curr_nt->next;
		}
		
		selection_action = (x->r_ob.num_selecteditems == count_notes_in_region) ? k_SINGLE_SELECTION : k_MULTIPLE_SELECTION;
		
		if (count_notes_in_region + 1 == ((t_note *)clicked_ptr)->parent->num_notes) {
			if (is_editable((t_notation_obj *)x, k_SELECTION, selection_action)) {
				curr_nt = ((t_note *)clicked_ptr)->parent->firstnote;
				while (curr_nt) { // how many notes are inside the selection?
					notation_item_delete_from_selection((t_notation_obj *) x, (t_notation_item *)curr_nt);
					curr_nt = curr_nt->next;
				}
			}
			clicked_ptr = ((t_note *)clicked_ptr)->parent;
			clicked_obj = k_CHORD;
		}
	}
	
	// mouse change??
	if (clicked_obj == k_NOTE || clicked_obj == k_CHORD) {
		if (modifiers & eAltKey && !(modifiers & eCommandKey)) {
			bach_set_cursor((t_object *)x, &x->r_ob.j_mouse_cursor, patcherview, BACH_CURSOR_COPY);
		}
	}
	
	
	set_mousedown((t_notation_obj *) x, clicked_ptr, clicked_obj);

	// handle selections
	if (is_editable((t_notation_obj *)x, k_SELECTION, selection_action) && x->r_ob.j_mousedown_ptr && (x->r_ob.j_mousedown_obj_type == k_NOTE || x->r_ob.j_mousedown_obj_type == k_DURATION_LINE || x->r_ob.j_mousedown_obj_type == k_CHORD || x->r_ob.j_mousedown_obj_type == k_ARTICULATION)){
		
		if (clicked_obj == k_DURATION_LINE) {
			clicked_ptr = ((t_duration_line *)clicked_ptr)->owner;
			clicked_obj = k_NOTE;
		}
		
		if (notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)clicked_ptr)) {
			// this case will be handled at the mouseup: if we won't drag, we'll delete the element from the selection
			if (!(modifiers & eShiftKey))
				x->r_ob.j_clicked_obj_has_been_selected = true;
		} else if (clicked_obj == k_NOTE && notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)((t_note *)clicked_ptr)->parent)) {
			// this case will be handled at the mouseup too (if we won't drag)
			// particular case of a chord selected and we click on a note, just to deselect the note. but to accomplish this we have to:
			// 1) deselect the chord; 2) select all the notes but this one
		} else { // we add it to the selection. We do that IMMEDIATELY. Preselection becomes IMMEDIATELY the selection
			if (!(modifiers & eShiftKey))
				clear_selection((t_notation_obj *) x);
			notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)clicked_ptr);
			// move the preselection to the overall selection
			move_preselecteditems_to_selection((t_notation_obj *) x, k_SELECTION_MODE_INVOLUTIVE, !(modifiers & eControlKey), false);
			x->r_ob.j_clicked_obj_has_been_selected = true;
		}
        
	} else if ((x->r_ob.j_mousedown_ptr && 
				(x->r_ob.j_mousedown_obj_type == k_DURATION_TAIL || x->r_ob.j_mousedown_obj_type == k_PITCH_BREAKPOINT
				 || x->r_ob.j_mousedown_obj_type == k_MARKER || x->r_ob.j_mousedown_obj_type == k_VOICE || x->r_ob.j_mousedown_obj_type == k_LYRICS || x->r_ob.j_mousedown_obj_type == k_DYNAMICS))) {
		if (notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)x->r_ob.j_mousedown_ptr)) {
			// this case will be handled at the mouseup: if we won't drag and shift is pressed, we'll delete the element from the selection
		} else {
			if (is_editable((t_notation_obj *)x, k_SELECTION, k_SINGLE_SELECTION) || is_editable((t_notation_obj *)x, k_SELECTION, k_MULTIPLE_SELECTION)) 
				if (!(modifiers & eShiftKey))
					clear_selection((t_notation_obj *) x);
			if (is_editable((t_notation_obj *)x, k_SELECTION, k_SINGLE_SELECTION)) {
				notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)clicked_ptr);
				// move the preselection to the overall selection x->r_ob.selection_type
				move_preselecteditems_to_selection((t_notation_obj *) x, k_SELECTION_MODE_INVOLUTIVE, !(modifiers & eControlKey), false);
				x->r_ob.j_clicked_obj_has_been_selected = true;
			}
		}
                    
	} else if (x->r_ob.j_mousedown_obj_type != k_SCROLLBAR && x->r_ob.j_mousedown_obj_type != k_VSCROLLBAR) {
		if (is_editable((t_notation_obj *)x, k_SELECTION, k_SINGLE_SELECTION) || is_editable((t_notation_obj *)x, k_SELECTION, k_MULTIPLE_SELECTION)) 
			if (!(modifiers & eShiftKey))
				clear_selection((t_notation_obj *) x);
	}
	
	// step back to note clicked_ptr
	if (clicked_ptr && clicked_obj == k_CHORD && clicked_ptr_note)
		set_mousedown((t_notation_obj *) x, clicked_ptr_note, k_NOTE);
    
	if (need_set_selection_dragging_velocity) 
		set_selection_dragging_velocity((t_notation_obj *) x);

	unlock_general_mutex((t_notation_obj *)x);	
	
	if (!clicked_ptr && modifiers == eCommandKey && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_CREATION)) { // add chord with 1 note!
		double argv[2], onset; 
		argv[0] = round(((double)getdomain((t_notation_obj *) x))/CONST_DEFAULT_DURATION);
		argv[1] = yposition_to_mc((t_notation_obj *)x, pt.y, NULL, NULL);
		onset = xposition_to_onset((t_notation_obj *) x,pt.x, yposition_to_systemnumber((t_notation_obj *) x, pt.y));
//		post("onset: %ld, addchord_from_values: %ld, %ld", onset, argv[0], argv[1]);
		if (onset >= 0.) {
			// we gotta find in WHICH rollvoice the user has clicked!
            t_chord *temp;
			long voicenum = yposition_to_voicenumber((t_notation_obj *)x, pt.y, -1, k_VOICEENSEMBLE_INTERFACE_ACTIVE);
            if (voicenum < 0)
                voicenum = yposition_to_voicenumber((t_notation_obj *)x, pt.y, -1, k_VOICEENSEMBLE_INTERFACE_ANY);
			
            lock_general_mutex((t_notation_obj *)x);
			
            temp = addchord_from_values(x, voicenum, 1, onset, -1, 2, argv, NULL, NULL, 0, NULL, false, 0, NULL, true);
            if (temp && temp->firstnote)
                set_slots_values_to_note_from_llll((t_notation_obj *)x, temp->firstnote, x->r_ob.default_noteslots);

            create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)temp, k_UNDO_MODIFICATION_DELETE);
			
			if (temp){
				if (is_editable((t_notation_obj *)x, k_SELECTION, x->r_ob.num_selecteditems > 0 ? k_MULTIPLE_SELECTION : k_SINGLE_SELECTION))
					notation_item_add_to_selection((t_notation_obj *) x, (t_notation_item *)temp);
				if (x->r_ob.snap_pitch_to_grid_when_editing) {
					t_note *nt;
					for (nt = temp->firstnote; nt; nt = nt->next){
						note_compute_approximation((t_notation_obj *) x, nt);
						snap_pitch_to_grid_for_note((t_notation_obj *) x, nt);
					}
				}
				if (x->r_ob.snap_onset_to_grid_when_editing)
					snap_onset_to_grid_for_chord((t_notation_obj *) x, temp, NULL);
				if (x->r_ob.snap_tail_to_grid_when_editing)
					snap_tail_to_grid_for_note((t_notation_obj *) x, temp->firstnote, NULL);
				constraint_midicents_depending_on_editing_ranges((t_notation_obj *)x, &temp->firstnote->midicents, voicenum); 
			}
			
			x->r_ob.item_changed_at_mousedown = 1;
			unlock_general_mutex((t_notation_obj *)x);	
//			x->r_ob.changed_while_dragging = true;
			handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_ADD_NOTE);
			return;
		}
	}
	
	if (need_send_changed_bang)
		handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_SEND_BANG, k_UNDO_OP_UNKNOWN);
	else	
		handle_change_selection((t_notation_obj *)x);

}


// returns the nth (0-based!!!) note of a chord
t_rollvoice* nth_rollvoice(t_roll *x, long n){ 
	t_rollvoice *curr = x->firstvoice; long i;
	if (n<0) return curr;
	if (n >= x->r_ob.num_voices) n = x->r_ob.num_voices;
	for (i=0; i<n; i++)
		curr = curr->next;
	return curr;
	// to be improved: if the note# is > n/2, pass the list the other way round!
}

void send_subroll_values_as_llll(t_roll *x, t_llll* whichvoices, double start_ms, double end_ms, t_llll *what_to_dump, char subroll_type){
	t_llll* out_llll = get_subroll_values_as_llll(x, whichvoices, start_ms, end_ms, what_to_dump, subroll_type);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 0, out_llll);
	llll_free(out_llll);
}

void send_roll_values_as_llll(t_roll *x, e_header_elems send_what)
{
	t_llll* out_llll = get_roll_values_as_llll(x, k_CONSIDER_ALL_NOTES, send_what, true, false);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 0, out_llll);
	llll_free(out_llll);
}

void send_onsets_values_as_llll(t_roll *x){
	t_llll* out_llll = get_onsets_values_as_llll(x);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 1, out_llll);
	llll_free(out_llll);
}

void send_durations_values_as_llll(t_roll *x){
	t_llll* out_llll = get_durations_values_as_llll(x);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 3, out_llll);
	llll_free(out_llll);
}

void send_cents_values_as_llll(t_roll *x){
	t_llll* out_llll = get_cents_values_as_llll(x);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 2, out_llll);
	llll_free(out_llll);
}

void send_velocities_values_as_llll(t_roll *x){
	t_llll* out_llll = get_velocities_values_as_llll(x);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 4, out_llll);
	llll_free(out_llll);
}

void send_extras_values_as_llll(t_roll *x){
	t_llll* out_llll = get_extras_values_as_llll(x);
	llllobj_outlet_llll((t_object *) x, LLLL_OBJ_UI, 5, out_llll);
	llll_free(out_llll);
}


t_llll* get_subroll_values_as_llll(t_roll *x, t_llll* whichvoices, double start_ms, double end_ms, t_llll *what_to_dump, char subroll_type){
	t_llll* out_llll = llll_get();
	t_llll *midichannels = llll_get(); 
	t_llll *clefs = llll_get();
	t_llll *keys = llll_get();
	t_llll *voicenames = llll_get();
	t_llll *stafflines = llll_get();
	
	t_rollvoice *voice;
	char we_take_it[CONST_MAX_VOICES];
	char what_to_dump_is_empty = (what_to_dump && has_llll_symbols_in_first_level(what_to_dump)) ? false : true;
	
	lock_general_mutex((t_notation_obj *)x);	
	llll_appendsym(out_llll, _llllobj_sym_roll, 0, WHITENULL_llll); // "roll" message
	
	if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_slotinfo))
		llll_appendllll(out_llll, get_slotinfo_values_as_llll((t_notation_obj *) x, false, false, false), 0, WHITENULL_llll); // slotinfo

	if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_commands))
		llll_appendllll(out_llll, get_commands_values_as_llll((t_notation_obj *) x, k_CONSIDER_FOR_SUBDUMPING), 0, WHITENULL_llll); // command

	llll_appendsym(clefs, _llllobj_sym_clefs, 0, WHITENULL_llll);
	llll_appendsym(keys, _llllobj_sym_keys, 0, WHITENULL_llll);
	llll_appendsym(midichannels, _llllobj_sym_midichannels, 0, WHITENULL_llll);
	llll_appendsym(voicenames, _llllobj_sym_voicenames, 0, WHITENULL_llll);
	llll_appendsym(stafflines, _llllobj_sym_stafflines, 0, WHITENULL_llll);

	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) { // append chord lllls
		// check if we have to take the voice
		we_take_it[CLAMP(voice->v_ob.number, 0, CONST_MAX_VOICES - 1)] = 0;
		if (whichvoices->l_size == 0)
			we_take_it[CLAMP(voice->v_ob.number, 0, CONST_MAX_VOICES - 1)] = 1;
		else {
			t_llllelem* elem;
			for (elem = whichvoices->l_head; elem; elem = elem->l_next) {
				if (hatom_gettype(&elem->l_hatom) == H_LONG) {
					if (hatom_getlong(&elem->l_hatom) == voice->v_ob.number + 1) {
						we_take_it[CLAMP(voice->v_ob.number, 0, CONST_MAX_VOICES - 1)] = 1;
						break;
					}
				}
			}
		}
		
		if (we_take_it[voice->v_ob.number]) {
			llll_appendlong(midichannels, voice->v_ob.midichannel, 0, WHITENULL_llll);
			llll_appendsym(clefs, x->r_ob.clefs_as_symlist[voice->v_ob.number], 0, WHITENULL_llll);
			llll_appendsym(keys, x->r_ob.keys_as_symlist[voice->v_ob.number], 0, WHITENULL_llll);
			llll_append_notation_item_name(voicenames, (t_notation_item *)voice);
//			llll_appendsym(voicenames, voice->v_ob.r_it.name, 0, WHITENULL_llll);
			llll_append(stafflines, get_voice_stafflines_as_llllelem((t_notation_obj *)x, (t_voice *)voice), WHITENULL_llll);
		}
		voice = voice->next;
	}

	if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_clefs))
		llll_appendllll(out_llll, clefs, 0, WHITENULL_llll); // clefs
	else
		llll_free(clefs);
		
	if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_keys)) 
		llll_appendllll(out_llll, keys, 0, WHITENULL_llll); // keys
	else
		llll_free(keys);
	
	if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_voicenames)) 
		llll_appendllll(out_llll, voicenames, 0, WHITENULL_llll); // voicenames
	else
		llll_free(voicenames);
	
	if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_markers)) {
		if (x->r_ob.firstmarker) // markers
			llll_appendllll(out_llll, get_markers_as_llll((t_notation_obj *) x, 1, start_ms, end_ms, false, k_CONSIDER_FOR_SUBDUMPING, 0), 0, WHITENULL_llll); 
	}
	
	if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_midichannels)) 
		llll_appendllll(out_llll, midichannels, 0, WHITENULL_llll); // midichannels
	else
		llll_free(midichannels);

	if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_stafflines)) 
		llll_appendllll(out_llll, stafflines, 0, WHITENULL_llll); // stafflines
	else
		llll_free(stafflines);
	

    
    if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_articulationinfo))
        llll_appendllll(out_llll, get_articulationinfo_as_llll((t_notation_obj *)x));
    
    if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_noteheadinfo))
        llll_appendllll(out_llll, get_noteheadinfo_as_llll((t_notation_obj *)x));

    
    if (what_to_dump_is_empty || is_symbol_in_llll_first_level(what_to_dump, _llllobj_sym_body)) {
		voice = x->firstvoice;
		while (voice && (voice->v_ob.number < x->r_ob.num_voices)) { // append chord lllls
			if (we_take_it[voice->v_ob.number])
				llll_appendllll(out_llll, get_subvoice_values_as_llll(x, voice, start_ms, end_ms, subroll_type), 0, WHITENULL_llll);
			
			voice = voice->next;
		}
	}
    

	unlock_general_mutex((t_notation_obj *)x);

	return out_llll;
}

// dump_what is a combination of e_header_elems
// for_what is typically either k_CONSIDER_ALL_NOTES, k_CONSIDER_FOR_SAVING (which should act exactly the same) or k_CONSIDER_FOR_UNDO (only for undo cases, which will also save the ID of elements)
t_llll* get_roll_values_as_llll(t_roll *x, e_data_considering_types for_what, e_header_elems dump_what, char also_lock_general_mutex, char explicitly_get_also_default_stuff) { //char get_clefs, char get_keys, char get_markers, char get_slotinfo, char get_commands, char get_midichannels){

// get all the information concerning the roll and put it in a llll

// the output is given as a long text-like llll formatted as
// roll (slotinfo ( INFOSLOT ) ( INFOSLOT ) ( INFOSLOT ) ...) (commands ( CMD ) ( CMD ) ... ) (midichannels ch1 ch2 ch3 ...)   (   ROLLVOICE  1   )    (   ROLLVOICE  2  ) 
//
// each ( ROLLVOICE ) has the form            ( ( onset ( NOTE1 ) ( NOTE2 ) ( NOTE3 ) ... flag) ( onset ( NOTE1 ) ( NOTE2 ) ( NOTE3 ) ... flag) ...  flag )
//												        		  C H O R D   1								C H O R D   2
//
// each ch1, ch2, ch3... refears to the midichannels of the ROLLVOICE1, ROLLVOICE2, ROLLVOICE3...
//
// each ( INFOSLOT ) has the form   
// (slotnumber slotname slottype slotmin slotmax optional:slotkey)
//
// each ( CMD ) has the form   
// (commandnumber command_for_notes command_for_chords commandkey)
//
// each ( NOTE# ) has the form 
// ( duration midicents velocity 
//				  ( graphic screen_midicents  screen_accidental )
//				  ( breakpoints  (ms1 mc1 slope1) (ms2 mc2 slope2) (ms3 mc3 slope3)...)
//				  ( slots (slot# SLOT_VALUES) (slot# SLOT2_VALUES) ...)
// )
//
// where graphic, breakpoints and slot are called extras, so the form is more of a ( midicents duration velocity EXTRA1 EXTRA2 EXTRA3 ), and all the EXTRAS are optional 
//
// where each SLOT#_VALUES has the form
//					if slottype = "function":    (x1 y1 slope1) (x2 y2 slope2) (x3 y3 slope3)...
//					if slottype = "long" or "double": number
//					if slottype = "text", we have as arguments: "array_of_characters"
//					if slottype = "filelist", we have as arguments:  "filepath1" "filepath2" ...  "lastfilepath" active_file#

	t_llll* out_llll = llll_get();
	t_rollvoice *voice;
	
	llll_appendsym(out_llll, _llllobj_sym_roll, 0, WHITENULL_llll); // "roll" message
	
	if (also_lock_general_mutex)
		lock_general_mutex((t_notation_obj *)x);	

	llll_chain(out_llll, get_notation_obj_header_as_llll((t_notation_obj *)x, dump_what, false, explicitly_get_also_default_stuff, for_what == k_CONSIDER_FOR_UNDO, for_what));
	
	if (dump_what & k_HEADER_BODY) {
		voice = x->firstvoice;
		while (voice && (voice->v_ob.number < x->r_ob.num_voices)) { // append chord lllls
			llll_appendllll(out_llll, get_rollvoice_values_as_llll(x, voice, for_what), 0, WHITENULL_llll);	
			voice = voice->next;
		}
	}
	
	if (also_lock_general_mutex)
		unlock_general_mutex((t_notation_obj *)x);	

	return out_llll;
}

double get_rollchord_max_duration(t_roll *x, t_chord *chord){
	t_note *nt;
	double best_duration = 0;
	
	if (!chord) 
		return 0;
	
	for (nt = chord->firstnote; nt; nt = nt->next)
		if (nt->duration > best_duration)
			best_duration = nt->duration;

	return best_duration;
}

// subroll_type = 0 : standard (partial notes also)
// subroll_type = 1 : only head of notes within the region matters!
t_llll* get_subvoice_values_as_llll(t_roll *x, t_rollvoice *voice, double start_ms, double end_ms, char subroll_type){
	t_llll* out_llll = llll_get();
	t_chord *temp_chord = voice->firstchord;
	while (temp_chord) { // append chord lllls
		double max_duration = get_rollchord_max_duration(x, temp_chord);
		t_llll* to_append = NULL;
		if (subroll_type == 1) {
			if (temp_chord->onset >= start_ms && temp_chord->onset <= end_ms) {
				to_append = get_rollchord_values_as_llll((t_notation_obj *) x, temp_chord, k_CONSIDER_FOR_SUBDUMPING);
			}
		} else {
			if (temp_chord->onset >= start_ms && temp_chord->onset + max_duration <= end_ms) {
				to_append = get_rollchord_values_as_llll((t_notation_obj *) x, temp_chord, k_CONSIDER_FOR_SUBDUMPING);
			} else if ((temp_chord->onset >= start_ms && temp_chord->onset <= end_ms) || 
					   (temp_chord->onset + max_duration >= start_ms && temp_chord->onset + max_duration <= end_ms) ||
					   (temp_chord->onset < start_ms && temp_chord->onset + max_duration > end_ms)) {
				to_append = get_rollpartialchord_values_as_llll((t_notation_obj *) x, temp_chord, k_CONSIDER_FOR_SUBDUMPING, start_ms, end_ms);
			}
		}
		
		if (to_append) {
			hatom_setdouble(&to_append->l_head->l_hatom, hatom_getdouble(&to_append->l_head->l_hatom) - start_ms);	// we change the chord onset, with -start_ms
			llll_appendllll(out_llll, to_append, 0, WHITENULL_llll);	// we append the list
		}
		
		temp_chord = temp_chord->next;
	}
	return out_llll;
}

t_llll* get_rollvoice_values_as_llll(t_roll *x, t_rollvoice *voice, e_data_considering_types for_what){
	t_llll* out_llll = llll_get();
	t_chord *temp_chord = voice->firstchord;
	
	while (temp_chord) { // append chord lllls
		llll_appendllll(out_llll, get_rollchord_values_as_llll((t_notation_obj *) x, temp_chord, for_what), 0, WHITENULL_llll);	
		temp_chord = temp_chord->next;
	}

	if (for_what != k_CONSIDER_FOR_EXPORT_OM && for_what != k_CONSIDER_FOR_EXPORT_PWGL)
        llll_append_notationitem_flag((t_notation_obj *) x, out_llll, (t_notation_item *)voice);

	if (for_what == k_CONSIDER_FOR_UNDO) 
		llll_appendllll(out_llll, get_ID_as_llll((t_notation_item *) voice), 0, WHITENULL_llll);

	return out_llll;
}

t_llll* get_onsets_values_as_llll(t_roll *x){
	t_llll* out_llll = llll_get();
	t_rollvoice *voice;
	lock_general_mutex((t_notation_obj *)x);	
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) { // append chord lllls
		llll_appendllll(out_llll, get_voice_onsets_values_as_llll(voice), 0, WHITENULL_llll);	
		voice = voice->next;
	}
	unlock_general_mutex((t_notation_obj *)x);	
	return out_llll;
}

t_llll* get_cents_values_as_llll(t_roll *x){
	t_llll* out_llll = llll_get();
	t_rollvoice *voice;
	lock_general_mutex((t_notation_obj *)x);	
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		llll_appendllll(out_llll, get_voice_cents_values_as_llll(x, voice), 0, WHITENULL_llll);
		voice = voice->next;
	}
	unlock_general_mutex((t_notation_obj *)x);	
	return out_llll;
}

t_llll* get_durations_values_as_llll(t_roll *x){
	t_llll* out_llll = llll_get();
	t_rollvoice *voice;
	lock_general_mutex((t_notation_obj *)x);	
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		llll_appendllll(out_llll, get_voice_durations_values_as_llll(voice), 0, WHITENULL_llll);	
		voice = voice->next;
	}
	unlock_general_mutex((t_notation_obj *)x);	
	return out_llll;
}

t_llll* get_velocities_values_as_llll(t_roll *x){
	t_llll* out_llll = llll_get();
	t_rollvoice *voice;
	lock_general_mutex((t_notation_obj *)x);	
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		llll_appendllll(out_llll, get_voice_velocities_values_as_llll(voice), 0, WHITENULL_llll);	
		voice = voice->next;
	}
	unlock_general_mutex((t_notation_obj *)x);	
	return out_llll;
}

t_llll* get_pixel_values_as_llll(t_roll *x){
	t_llll* out_llll = llll_get();
	t_rollvoice *voice;
	lock_general_mutex((t_notation_obj *)x);	
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		llll_appendllll(out_llll, get_voice_pixel_values_as_llll(x, voice), 0, WHITENULL_llll);	
		voice = voice->next;
	}
	unlock_general_mutex((t_notation_obj *)x);	
	return out_llll;
}

t_llll* get_voice_pixel_values_as_llll(t_roll *x, t_rollvoice *voice){
// get all the information concerning the pixels and put it in a llll, just for a voice

// the output is given as a long text-like llll formatted as
// ((xpixel_chord1 pixel_duration_chord1)  (pixel_chord2 pixel_duration_chord2) ... )

	t_llll* out_llll = llll_get();
	t_chord *temp_chord; t_note *temp_note;
	for (temp_chord = voice->firstchord; temp_chord; temp_chord = temp_chord->next) {
		t_llll *chord_llll = llll_get();
		t_llll *notes_pixel_durations_llll = llll_get();
		t_llll *notes_y_pixel_pos = llll_get();
		t_llll *accidentals_x_pixel_pos = llll_get();
		long system = -1;
		double this_chord_pixel_start = onset_to_xposition((t_notation_obj *) x, temp_chord->onset, &system);
		llll_appenddouble(chord_llll, this_chord_pixel_start, 0, WHITENULL_llll); // pixel start
		for (temp_note = temp_chord->firstnote; temp_note; temp_note = temp_note->next) { // pixel duration
			llll_appenddouble(notes_pixel_durations_llll, onset_to_xposition((t_notation_obj *) x, temp_chord->onset + temp_note->duration, &system) - this_chord_pixel_start, 0, WHITENULL_llll);	
			llll_appenddouble(notes_y_pixel_pos, mc_to_yposition((t_notation_obj *) x, note_get_screen_midicents(temp_note), (t_voice *) voice), 0, WHITENULL_llll);
			if (note_get_screen_accidental(temp_note).r_num != 0)
				llll_appenddouble(accidentals_x_pixel_pos, this_chord_pixel_start + temp_note->accidental_stem_delta_ux * x->r_ob.zoom_y + 
							  x->r_ob.accidentals_typo_preferences.ux_shift * x->r_ob.zoom_y - 
							  get_accidental_uwidth((t_notation_obj *) x, note_get_screen_accidental(temp_note), false) * x->r_ob.zoom_y, 0, WHITENULL_llll);
			else 
				llll_appenddouble(accidentals_x_pixel_pos, this_chord_pixel_start + 
								  get_notehead_ux_shift((t_notation_obj *) x, temp_note) * x->r_ob.zoom_y + temp_note->notecenter_stem_delta_ux * x->r_ob.zoom_y -
								  (temp_note->notehead_uwidth * x->r_ob.zoom_y), 0, WHITENULL_llll); 
		}
		llll_appendllll(chord_llll, notes_pixel_durations_llll, 0, WHITENULL_llll);	
		llll_appendllll(chord_llll, notes_y_pixel_pos, 0, WHITENULL_llll);	
		llll_appendllll(chord_llll, accidentals_x_pixel_pos, 0, WHITENULL_llll);	
		llll_appendllll(out_llll, chord_llll, 0, WHITENULL_llll);	
	}
	return out_llll;
}


t_llll* get_voice_onsets_values_as_llll(t_rollvoice *voice){
// get all the information concerning the onsets and put it in a llll, just for a voice

// the output is given as a long text-like llll formatted as
// (onset_chord1  onset_chord2  onset_chord3 ... )

	t_llll* out_llll = llll_get();
	t_chord *temp_chord = voice->firstchord;
	while (temp_chord) { // append chord lllls
		llll_appenddouble(out_llll, temp_chord->onset, 0, WHITENULL_llll);	
		temp_chord = temp_chord->next;
	}

	return out_llll;
}

t_llll* get_voice_cents_values_as_llll(t_roll *x, t_rollvoice *voice){
// get all the information concerning the onsets and put it in a llll

// the output is given as a long text-like llll formatted as
// ((midicents_note1   midicents_note2 ....)   (midicents_note1    midicents_note2 ....)  ...)
//              C H O R D   1                              C H O R D     2

	t_llll* out_llll = llll_get();
	t_chord *temp_chord = voice->firstchord;
	while (temp_chord) { // append chord lllls
		t_note *temp_note = temp_chord->firstnote;
		t_llll* in_llll = llll_get();
		while (temp_note) { // append chord lllls
            note_appendpitch_to_llll_for_separate_syntax((t_notation_obj *)x, in_llll, temp_note);
			temp_note = temp_note->next;
		}
		llll_appendllll(out_llll, in_llll, 0, WHITENULL_llll);	
		temp_chord = temp_chord->next;
	}

	return out_llll;
}

t_llll* get_voice_durations_values_as_llll(t_rollvoice *voice){
// get all the information concerning the onsets and put it in a llll

// the output is given as a long text-like llll formatted as
// ((duration_note1   duration_note2 ....)   (duration_note1    duration_note2 ....)  ...)
//              C H O R D   1                              C H O R D     2

	t_llll* out_llll = llll_get();
	t_chord *temp_chord = voice->firstchord;
	while (temp_chord) { // append chord lllls
		t_note *temp_note = temp_chord->firstnote;
		t_llll* in_llll = llll_get();
		while (temp_note) { // append chord lllls
			llll_appenddouble(in_llll, temp_note->duration, 0, WHITENULL_llll);	
			temp_note = temp_note->next;
		}
		llll_appendllll(out_llll, in_llll, 0, WHITENULL_llll);	
		temp_chord = temp_chord->next;
	}

	return out_llll;
}

t_llll* get_voice_velocities_values_as_llll(t_rollvoice *voice){
// get all the information concerning the onsets and put it in a llll

// the output is given as a long text-like llll formatted as
// ((velocity_note1   velocity_note2 ....)   (velocity_note1    velocity_note2 ....)  ...)
//              C H O R D   1                              C H O R D     2

	t_llll* out_llll = llll_get();
	t_chord *temp_chord = voice->firstchord;
	while (temp_chord) { // append chord lllls
		t_note *temp_note = temp_chord->firstnote;
		t_llll* in_llll = llll_get();
		while (temp_note) { // append chord lllls
			llll_appendlong(in_llll, temp_note->velocity, 0, WHITENULL_llll);	
			temp_note = temp_note->next;
		}
		llll_appendllll(out_llll, in_llll, 0, WHITENULL_llll);	
		temp_chord = temp_chord->next;
	}

	return out_llll;
}

t_llll* get_extras_values_as_llll(t_roll *x){
// get all the information concerning the onsets and put it in a llll
// TO DO!
// the output is given as a long text-like llll formatted as
// ((velocity_note1   velocity_note2 ....)   (velocity_note1    velocity_note2 ....)  ...)
//              C H O R D   1                              C H O R D     2
	t_rollvoice *voice;
	t_llll* out_llll = llll_get();
//	t_llll* graphic_llll = llll_get();
	t_llll* breakpoints_llll = llll_get();
	t_llll* slots_llll = llll_get();
	lock_general_mutex((t_notation_obj *)x);
//	llll_appendsym(graphic_llll, _llllobj_sym_graphic, 0, WHITENULL_llll);
	llll_appendsym(breakpoints_llll, _llllobj_sym_breakpoints, 0, WHITENULL_llll);
	llll_appendsym(slots_llll, _llllobj_sym_slots, 0, WHITENULL_llll);

	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
//		t_llll* graphic_voicellll = llll_get();
		t_llll* breakpoints_voicellll = llll_get();
		t_llll* slots_voicellll = llll_get();
		
		t_chord *temp_chord = voice->firstchord;
		while (temp_chord) { // append chord lllls
			t_note *temp_note = temp_chord->firstnote;
//			t_llll* graphic_subllll = llll_get();
			t_llll* breakpoints_subllll = llll_get();
			t_llll* slots_subllll = llll_get();
			while (temp_note) { // append chord lllls
//				llll_appendllll(graphic_subllll, note_get_graphic_values_no_router_as_llll((t_notation_obj *) x, temp_note), 0, WHITENULL_llll);
				llll_appendllll(breakpoints_subllll, note_get_breakpoints_values_no_router_as_llll((t_notation_obj *) x, temp_note), 0, WHITENULL_llll);
				llll_appendllll(slots_subllll, note_get_slots_values_no_header_as_llll((t_notation_obj *) x, temp_note, false), 0, WHITENULL_llll);
				temp_note = temp_note->next;
			}
//			llll_appendllll(graphic_voicellll, graphic_subllll, 0, WHITENULL_llll);
			llll_appendllll(breakpoints_voicellll, breakpoints_subllll, 0, WHITENULL_llll);
			llll_appendllll(slots_voicellll, slots_subllll, 0, WHITENULL_llll);
			temp_chord = temp_chord->next;
		}
		
//		llll_appendllll(graphic_llll, graphic_voicellll, 0, WHITENULL_llll);
		llll_appendllll(breakpoints_llll, breakpoints_voicellll, 0, WHITENULL_llll);	
		llll_appendllll(slots_llll, slots_voicellll, 0, WHITENULL_llll);	
		voice = voice->next;
	}
//	llll_appendllll(out_llll, graphic_llll, 0, WHITENULL_llll);
	llll_appendllll(out_llll, breakpoints_llll, 0, WHITENULL_llll);	
	llll_appendllll(out_llll, slots_llll, 0, WHITENULL_llll);	
	
	unlock_general_mutex((t_notation_obj *)x);	
	return out_llll;
}

void snap_onset_tail_pitch_to_grid_for_selection_if_needed(t_roll *x)
{
    if (x->r_ob.snap_onset_to_grid_when_editing && x->r_ob.j_dragging_direction != -1)
        snap_onset_to_grid_for_selection(x);

    if (x->r_ob.snap_markers_to_grid_when_editing && x->r_ob.j_dragging_direction != -1)
        snap_markers_to_grid_for_selection(x);
    
    if (x->r_ob.snap_tail_to_grid_when_editing && x->r_ob.j_dragging_direction != -1)
        snap_tail_to_grid_for_selection((t_notation_obj *)x);
    
    if (x->r_ob.snap_pitch_to_grid_when_editing && x->r_ob.j_dragging_direction != 1)
        snap_pitch_to_grid_for_selection((t_notation_obj *)x);
}

void roll_mouseup(t_roll *x, t_object *patcherview, t_pt pt, long modifiers) {
	char there_are_free_undo_ticks;

	lock_general_mutex((t_notation_obj *)x);	
	handle_mouseup_in_bach_inspector((t_notation_obj *) x, &x->r_ob.m_inspector, patcherview, pt);
	handle_slot_mouseup((t_notation_obj *)x, patcherview, pt, modifiers);
	unlock_general_mutex((t_notation_obj *)x);	
	
	there_are_free_undo_ticks = are_there_free_undo_ticks((t_notation_obj *) x, false);
	
	x->r_ob.j_mouse_is_down = false;
	x->r_ob.j_isdragging = false;
	
	if (x->r_ob.changed_while_dragging && x->r_ob.active_slot_num < 0)
        snap_onset_tail_pitch_to_grid_for_selection_if_needed(x);
    
    if (x->r_ob.need_snap_some_nonselected_items) {
        snap_to_grid_for_flagged_items(x, x->r_ob.snap_onset_to_grid_when_editing, x->r_ob.snap_tail_to_grid_when_editing);
        x->r_ob.need_snap_some_nonselected_items = false;
    }
    

	if (there_are_free_undo_ticks)
		handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_MOUSEDRAG_CHANGE);

	bach_set_cursor((t_object *)x, &x->r_ob.j_mouse_cursor, patcherview, BACH_CURSOR_DEFAULT);
	
	if (!x->r_ob.j_mouse_hasbeendragged) { // mouse hasn't been dragged
		if (!x->r_ob.j_clicked_obj_has_been_selected && modifiers & eShiftKey) {
			if ((x->r_ob.j_mousedown_ptr) && ((x->r_ob.j_mousedown_obj_type == k_NOTE) || (x->r_ob.j_mousedown_obj_type == k_CHORD))) {
				if (notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)x->r_ob.j_mousedown_ptr)) {
					notation_item_delete_from_selection((t_notation_obj *)x, (t_notation_item *)x->r_ob.j_mousedown_ptr);
				} else if (x->r_ob.j_mousedown_obj_type == k_NOTE && notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)((t_note *)x->r_ob.j_mousedown_ptr)->parent)) {
					// particular case of a chord selected and we click on a note, just to deselect the note. but to accomplish this we have to:
					// 1) deselect the chord; 2) select all the notes but this one
					t_chord *this_chord = ((t_note *)x->r_ob.j_mousedown_ptr)->parent;
					t_note *this_note;
					lock_general_mutex((t_notation_obj *)x);	
					notation_item_delete_from_selection((t_notation_obj *)x, (t_notation_item *)this_chord); // delete the chord from the selection
					for (this_note = this_chord->firstnote; this_note; this_note = this_note->next)
						if (this_note != x->r_ob.j_mousedown_ptr)
							notation_item_add_to_selection((t_notation_obj *) x, (t_notation_item *)this_note);
					unlock_general_mutex((t_notation_obj *)x);	
				}
			} else if (x->r_ob.j_mousedown_ptr && (x->r_ob.j_mousedown_obj_type == k_DURATION_TAIL || x->r_ob.j_mousedown_obj_type == k_PITCH_BREAKPOINT 
						|| x->r_ob.j_mousedown_obj_type == k_MARKER)) {
				lock_general_mutex((t_notation_obj *)x);	
				if (notation_item_is_selected((t_notation_obj *) x, (t_notation_item *)x->r_ob.j_mousedown_ptr)) 
					notation_item_delete_from_selection((t_notation_obj *)x, (t_notation_item *)x->r_ob.j_mousedown_ptr);
				unlock_general_mutex((t_notation_obj *)x);	
			}
		}
	}

	if (!there_are_free_undo_ticks && !x->r_ob.item_changed_at_mousedown && x->r_ob.j_mouse_hasbeendragged && ((x->r_ob.j_mousedown_obj_type >= k_DILATION_RECTANGLE_TOPLEFT_SQ && x->r_ob.j_mousedown_obj_type <= k_DILATION_RECTANGLE_MIDDLERIGHT_SQ) ||
		x->r_ob.j_mousedown_obj_type == k_DILATION_RECTANGLE_BODY)) {
		lock_general_mutex((t_notation_obj *)x);
		t_rollvoice *voice;
		t_chord *ch;
		for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next)
			for (ch = voice->firstchord; ch; ch = ch->next)
				if (ch->onset < 0)
					ch->onset = 0;
		check_all_chords_order(x);
		unlock_general_mutex((t_notation_obj *)x);
		recompute_total_length((t_notation_obj *)x);
	}

	if (!there_are_free_undo_ticks && !x->r_ob.item_changed_at_mousedown && x->r_ob.j_mousedown_obj_type == k_ZOOMING_REGION && x->r_ob.j_mousedown_point.x != pt.x) { 
		// new zoom
		double start_x, end_x;
		double start_ms, end_ms;
		double old_zoom, new_zoom;
		if (pt.x > x->r_ob.j_mousedown_point.x) {
			start_x = x->r_ob.j_mousedown_point.x;
			end_x = pt.x;
		} else {
			start_x = pt.x;
			end_x = x->r_ob.j_mousedown_point.x;
		}
		start_ms = xposition_to_onset((t_notation_obj *) x, start_x, 0);
		end_ms = xposition_to_onset((t_notation_obj *) x, end_x, 0);
		
		if (end_ms > start_ms) {
			getdomain((t_notation_obj *) x);
			old_zoom = x->r_ob.horizontal_zoom;
			if (modifiers & eShiftKey)
				new_zoom = old_zoom / (((double) x->r_ob.domain)/(end_ms-start_ms));
			else
				new_zoom = old_zoom * (((double) x->r_ob.domain)/(end_ms-start_ms));
			change_zoom((t_notation_obj *) x, new_zoom);
			x->r_ob.screen_ms_start = start_ms;
            x->r_ob.screen_ux_start = onset_to_unscaled_xposition((t_notation_obj *)x, x->r_ob.screen_ms_start);
			update_hscrollbar((t_notation_obj *)x, 2);
			send_domain(x, 6, NULL);
			invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
		}
	}

	if (!there_are_free_undo_ticks && !x->r_ob.item_changed_at_mousedown && (x->r_ob.j_mousedown_obj_type == k_LOOP_REGION) && (modifiers & eShiftKey)) { 
		double start_ms = x->r_ob.loop_region.start.position_ms;
		double end_ms = x->r_ob.loop_region.end.position_ms;
		t_chord *nearest_chord_start = get_nearest_chord(x, start_ms);
		t_chord *nearest_chord_end = get_nearest_chord(x, end_ms);
		if (nearest_chord_start)
			start_ms = nearest_chord_start->onset;
		if (nearest_chord_end)
			end_ms = nearest_chord_end->onset;
		set_loop_region_from_extremes(x, start_ms, end_ms);
		send_loop_region((t_notation_obj *) x, 6);
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	}
	
	// turning selection rectangle into a dilation rectangle
	if (!there_are_free_undo_ticks && !x->r_ob.item_changed_at_mousedown && (x->r_ob.j_mousedown_obj_type == k_REGION) && (x->r_ob.j_mousedown_point.x != pt.x || x->r_ob.j_mousedown_point.y != pt.y)) { 
		if (modifiers & eCommandKey){
			x->r_ob.dilation_rectangle.left_ms = MAX(0, xposition_to_onset((t_notation_obj *) x, x->r_ob.j_mousedown_point.x, yposition_to_systemnumber((t_notation_obj *) x, x->r_ob.j_mousedown_point.y)));
			x->r_ob.dilation_rectangle.right_ms = MAX(0, xposition_to_onset((t_notation_obj *) x, pt.x, yposition_to_systemnumber((t_notation_obj *) x, pt.y)));
            
            if (x->r_ob.dilation_rectangle.left_ms > x->r_ob.dilation_rectangle.right_ms)
                swap_doubles(&x->r_ob.dilation_rectangle.left_ms, &x->r_ob.dilation_rectangle.right_ms);
            
            if (x->r_ob.j_mousedown_point.y < pt.y) {
                x->r_ob.dilation_rectangle.top_mc = yposition_to_mc((t_notation_obj *)x, x->r_ob.j_mousedown_point.y, NULL, NULL);
                x->r_ob.dilation_rectangle.bottom_mc = yposition_to_mc((t_notation_obj *)x, pt.y, NULL, NULL);

                x->r_ob.dilation_rectangle.top_voice = (t_voice *)nth_rollvoice(x, yposition_to_voicenumber((t_notation_obj *)x, x->r_ob.j_mousedown_point.y, -1, k_VOICEENSEMBLE_INTERFACE_FIRST));
                x->r_ob.dilation_rectangle.bottom_voice = (t_voice *)nth_rollvoice(x, yposition_to_voicenumber((t_notation_obj *)x, pt.y, -1, k_VOICEENSEMBLE_INTERFACE_LAST));
            } else {
                x->r_ob.dilation_rectangle.bottom_mc = yposition_to_mc((t_notation_obj *)x, x->r_ob.j_mousedown_point.y, NULL, NULL);
                x->r_ob.dilation_rectangle.top_mc = yposition_to_mc((t_notation_obj *)x, pt.y, NULL, NULL);

                x->r_ob.dilation_rectangle.top_voice = (t_voice *)nth_rollvoice(x, yposition_to_voicenumber((t_notation_obj *)x, pt.y, -1, k_VOICEENSEMBLE_INTERFACE_FIRST));
                x->r_ob.dilation_rectangle.bottom_voice = (t_voice *)nth_rollvoice(x, yposition_to_voicenumber((t_notation_obj *)x, x->r_ob.j_mousedown_point.y, -1, k_VOICEENSEMBLE_INTERFACE_LAST));
            }
            
/*			if (x->r_ob.dilation_rectangle.top_voice->number > x->r_ob.dilation_rectangle.bottom_voice->number ||
				(x->r_ob.dilation_rectangle.top_voice->number == x->r_ob.dilation_rectangle.bottom_voice->number &&
				 x->r_ob.dilation_rectangle.top_mc < x->r_ob.dilation_rectangle.bottom_mc)) {
					t_voice *tempv = x->r_ob.dilation_rectangle.top_voice;
					double temp = x->r_ob.dilation_rectangle.top_mc;
					x->r_ob.dilation_rectangle.top_voice = x->r_ob.dilation_rectangle.bottom_voice;
					x->r_ob.dilation_rectangle.bottom_voice = tempv;
					x->r_ob.dilation_rectangle.top_mc = x->r_ob.dilation_rectangle.bottom_mc;
					x->r_ob.dilation_rectangle.bottom_mc = temp;
			} */
			x->r_ob.show_dilation_rectangle = true;
		} else {
			
			// accomplish a region selection
			x->r_ob.j_selected_region_ms1 = xposition_to_onset((t_notation_obj *) x, x->r_ob.j_mousedown_point.x, yposition_to_systemnumber((t_notation_obj *) x, x->r_ob.j_mousedown_point.y));
			x->r_ob.j_selected_region_mc1 = yposition_to_mc((t_notation_obj *)x, x->r_ob.j_mousedown_point.y, NULL, NULL);
			x->r_ob.j_selected_region_ms2 = xposition_to_onset((t_notation_obj *) x, pt.x, yposition_to_systemnumber((t_notation_obj *) x, pt.y));
			x->r_ob.j_selected_region_mc2 = yposition_to_mc((t_notation_obj *)x, pt.y, NULL, NULL);
			if (x->r_ob.j_selected_region_ms1 > x->r_ob.j_selected_region_ms2) {
				long temp = x->r_ob.j_selected_region_ms1;
				x->r_ob.j_selected_region_ms1 = x->r_ob.j_selected_region_ms2;
				x->r_ob.j_selected_region_ms2 = temp;
			}
			if (x->r_ob.j_selected_region_mc1 > x->r_ob.j_selected_region_mc2) {
				long temp = x->r_ob.j_selected_region_mc1;
				x->r_ob.j_selected_region_mc1 = x->r_ob.j_selected_region_mc2;
				x->r_ob.j_selected_region_mc2 = temp;
			}
		}
	}

	// move the preselection to the overall selection
	lock_general_mutex((t_notation_obj *)x);	
	move_preselecteditems_to_selection((t_notation_obj *) x, k_SELECTION_MODE_INVOLUTIVE, !(modifiers & eControlKey), false);
	unlock_general_mutex((t_notation_obj *)x);	
	
	handle_change_selection((t_notation_obj *)x);

	x->r_ob.j_mousedrag_copy_ptr = NULL;
    set_mousedown((t_notation_obj *)x, NULL, k_NONE, false);
	x->r_ob.j_dragging_direction = 0;
	
	// reset the dragged flags
	x->r_ob.j_mouse_hasbeendragged = 0;
	x->r_ob.j_selection_hasbeendragged = 0;
	x->r_ob.item_changed_at_mousedown = 0;
	
    notationobj_redraw((t_notation_obj *) x);
}

void roll_mousewheel(t_roll *x, t_object *view, t_pt pt, long modifiers, double x_inc, double y_inc){
    char res = 0;
    llll_format_modifiers(&modifiers, NULL);

	lock_general_mutex((t_notation_obj *)x);	
	res = handle_slot_mousewheel((t_notation_obj *) x, view, pt, modifiers, x_inc, y_inc);
	unlock_general_mutex((t_notation_obj *)x);	

	if (res)
		return;
	
	if (modifiers == eCommandKey || modifiers == eShiftKey + eCommandKey) { // change zoom
		double old_zoom = x->r_ob.horizontal_zoom;
		double new_zoom = old_zoom;
		double pt_ms = xposition_to_onset((t_notation_obj *) x, pt.x, 0);

		if (!is_editable((t_notation_obj *)x, k_ZOOMING_REGION, k_ELEMENT_ACTIONS_NONE)) return;

		new_zoom -= y_inc * CONST_Y_MOUSEWHEEL_FACTOR * (modifiers & eShiftKey ? CONST_FINER_FROM_KEYBOARD : 1.);
		if (new_zoom < 1) new_zoom = 1;
		change_zoom((t_notation_obj *) x, new_zoom);
		
		// we preserve pt.x position upon zoom
		x->r_ob.screen_ms_start = - (old_zoom / x->r_ob.horizontal_zoom) * (pt_ms - x->r_ob.screen_ms_start) + pt_ms;

		if (x->r_ob.screen_ms_start < 0) 
			x->r_ob.screen_ms_start = 0.;
 		
        x->r_ob.screen_ux_start = onset_to_unscaled_xposition((t_notation_obj *)x, x->r_ob.screen_ms_start);
		update_hscrollbar((t_notation_obj *)x, 2);

		send_domain(x, 6, NULL);
		invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
		
	} else { // simple scroll

		if (!is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE)) return;

		if (x->r_ob.show_hscrollbar) {
            double delta_x = - (x_inc * 25 * CONST_X_MOUSEWHEEL_FACTOR / CONST_X_SCALING) / x->r_ob.zoom_x;
            if (modifiers & eShiftKey && modifiers & eCommandKey)
                delta_x *= CONST_FINER_FROM_KEYBOARD;
            x->r_ob.screen_ms_start = MAX(0, x->r_ob.screen_ms_start + delta_x);
            redraw_hscrollbar((t_notation_obj *)x, 2);
            send_domain(x, 6, NULL);
            invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
        }

		if (x->r_ob.show_vscrollbar) {
			double delta_y = y_inc * CONST_Y_MOUSEWHEEL_FACTOR;
			if (modifiers & eShiftKey && modifiers & eCommandKey) 
				delta_y *= CONST_FINER_FROM_KEYBOARD;
			x->r_ob.vscrollbar_y -= delta_y;
			redraw_vscrollbar((t_notation_obj *)x, 0);
			//send_vdomain(x, 6, NULL);
			invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
		}
		
	} 
}


void roll_mousedoubleclick(t_roll *x, t_object *patcherview, t_pt pt, long modifiers) {
	// first of all: are we in a slot mode???? Cause if we are in a slot mode, we gotta handle that separately
	char changed = false;
	char clicked_slot;
	t_rollvoice *voice;
	
	if (x->r_ob.active_slot_num > -1 && !is_editable((t_notation_obj *)x, k_SLOT, k_ELEMENT_ACTIONS_NONE)) return;

	lock_general_mutex((t_notation_obj *)x);	
	clicked_slot = handle_slot_mousedoubleclick((t_notation_obj *) x, patcherview, pt, modifiers, &changed);
	
	if (clicked_slot) {
		unlock_general_mutex((t_notation_obj *)x);	
		handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_SLOT);
		return;
	}
	
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
		if (is_in_clef_shape((t_notation_obj *)x, pt.x, pt.y, (t_voice *)voice)) {
			clear_preselection((t_notation_obj *)x);
			preselect_elements_in_region_for_mouse_selection(x, 0, x->r_ob.length_ms, -1000, 16000, voice->v_ob.number, voice->v_ob.number);
			move_preselecteditems_to_selection((t_notation_obj *)x, k_SELECTION_MODE_FORCE_SELECT, false, false);
		}
	}
			
	if (x->r_ob.dblclick_sends_values) {
		unlock_general_mutex((t_notation_obj *)x);	
		evaluate_selection(x, 0, true);
	} else {
		// voice?
		t_rollvoice *voice;
		for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next) {
			if (is_in_voicename_shape((t_notation_obj *) x, pt.x, pt.y, (t_voice *) voice)){
				unlock_general_mutex((t_notation_obj *)x);	
				if (is_editable((t_notation_obj *)x, k_VOICE, k_MODIFICATION_NAME))
					start_editing_voicename((t_notation_obj *) x, patcherview, (t_voice *) voice);
				return;
			}
		}
		// markers?
		if (x->r_ob.firstmarker) {
			t_marker *marker;
			for (marker = x->r_ob.firstmarker; marker; marker = marker->next) {
				if (is_in_markername_shape(x, pt.x, pt.y, marker)){
					unlock_general_mutex((t_notation_obj *)x);	
					if (is_editable((t_notation_obj *)x, k_MARKER, k_MODIFICATION_NAME))
						start_editing_markername((t_notation_obj *) x, patcherview, marker, onset_to_xposition((t_notation_obj *)x, marker->position_ms, NULL) + 3 * x->r_ob.zoom_y);
					return;
				}
			}
		}
        
		// lyrics?
		if (x->r_ob.show_lyrics && x->r_ob.link_lyrics_to_slot > 0) {
			t_rollvoice *voice;
			t_chord *chord;
			for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
				for (chord = voice->firstchord; chord; chord = chord->next){
					if (chord->lyrics && chord->lyrics->label && is_in_chord_lyrics_shape((t_notation_obj *) x, chord, pt.x, pt.y)) {
						unlock_general_mutex((t_notation_obj *)x);	
						if (is_editable((t_notation_obj *)x, k_LYRICS, k_MODIFICATION_GENERIC))
							start_editing_lyrics((t_notation_obj *) x, patcherview, chord);
						return;
					}
				}
			}
		}

        // dynamics?
        if (x->r_ob.show_dynamics && x->r_ob.link_dynamics_to_slot > 0) {
            t_rollvoice *voice;
            t_chord *chord;
            for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
                for (chord = voice->firstchord; chord; chord = chord->next){
                    if (chord->dynamics && chord->dynamics->text && is_in_chord_dynamics_shape((t_notation_obj *) x, chord, pt.x, pt.y)) {
                        unlock_general_mutex((t_notation_obj *)x);
                        if (is_editable((t_notation_obj *)x, k_DYNAMICS, k_MODIFICATION_GENERIC))
                            start_editing_dynamics((t_notation_obj *) x, patcherview, chord);
                        return;
                    }
                }
            }
        }
    
    }
	unlock_general_mutex((t_notation_obj *)x);	
}


long roll_keyfilter(t_roll *x, t_object *patcherview, long *keycode, long *modifiers, long *textcharacter)
{
	t_atom arv;
	long rv = 1;
	long k = *keycode;
	
	if (k == JKEY_TAB) {
		if (x->r_ob.is_editing_type == k_LYRICS && x->r_ob.is_editing_chord && x->r_ob.is_editing_chord->firstnote) {
			char *text	= NULL;
			long size = 0;
			t_object *textfield = jbox_get_textfield((t_object *)x);
			object_method(textfield, gensym("gettextptr"), &text, &size);
			create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)x->r_ob.is_editing_chord, k_UNDO_MODIFICATION_CHANGE);
			set_textfield_info_to_lyrics_slot((t_notation_obj *) x, text);
			end_editing_textfield((t_notation_obj *) x);
			if (!((*modifiers) & eShiftKey) && x->r_ob.is_editing_chord->next)
				start_editing_lyrics((t_notation_obj *) x, patcherview, x->r_ob.is_editing_chord->next);
			else if (((*modifiers) & eShiftKey) && x->r_ob.is_editing_chord->prev)
				start_editing_lyrics((t_notation_obj *) x, patcherview, x->r_ob.is_editing_chord->prev);
			else
				object_method_typed(patcherview, gensym("endeditbox"), 0, NULL, &arv); 
			rv = 0;
        } else if (x->r_ob.is_editing_type == k_DYNAMICS && x->r_ob.is_editing_chord && x->r_ob.is_editing_chord->firstnote) {
            char *text	= NULL;
            long size = 0;
            t_object *textfield = jbox_get_textfield((t_object *)x);
            object_method(textfield, gensym("gettextptr"), &text, &size);
            create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)x->r_ob.is_editing_chord, k_UNDO_MODIFICATION_CHANGE);
            set_textfield_info_to_dynamics_slot((t_notation_obj *) x, text);
            end_editing_textfield((t_notation_obj *) x);
            if (!((*modifiers) & eShiftKey) && x->r_ob.is_editing_chord->next)
                start_editing_dynamics((t_notation_obj *) x, patcherview, x->r_ob.is_editing_chord->next);
            else if (((*modifiers) & eShiftKey) && x->r_ob.is_editing_chord->prev)
                start_editing_dynamics((t_notation_obj *) x, patcherview, x->r_ob.is_editing_chord->prev);
            else
                object_method_typed(patcherview, gensym("endeditbox"), 0, NULL, &arv); 
            rv = 0;
		}
	} else if (k == JKEY_ENTER || k == JKEY_RETURN || k == JKEY_ESC) {
		object_method_typed(patcherview, gensym("endeditbox"), 0, NULL, &arv); 
		rv = 0;		// don't pass those keys to uitextfield
	}
	return rv;
}


void roll_enter(t_roll *x)	// enter is triggerd at "endeditbox time"
{
	long size = 0;
	char *text = NULL;
	t_object *textfield;

	textfield = jbox_get_textfield((t_object *)x);
	object_method(textfield, gensym("gettextptr"), &text, &size);
	
	if (x->r_ob.is_editing_type == k_VOICENAME && x->r_ob.is_editing_voice_name >= 0) {
		long ac = 0;
		t_atom *av = NULL;
		atom_setparse_debug(&ac, &av, text);
		change_single_voicename_from_ac_av((t_notation_obj *) x, (t_voice *) nth_rollvoice(x, x->r_ob.is_editing_voice_name), ac, av, true);
		handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_VOICE_NAME);
		bach_freeptr(av);
	} else if (x->r_ob.is_editing_type == k_MARKERNAME && x->r_ob.is_editing_marker) {
		t_llll *names = llll_from_text_buf(text, 0);
		lock_markers_mutex((t_notation_obj *)x);;
		create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
		change_marker_names((t_notation_obj *) x, x->r_ob.is_editing_marker, names);
		handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_MARKER_NAME);
		unlock_markers_mutex((t_notation_obj *)x);;
		llll_free(names);
	} else if (x->r_ob.is_editing_type == k_TEXT_IN_SLOT) {
		t_llll *new_text_as_llll = llll_get();
		llll_appendsym(new_text_as_llll, gensym(text), 0, WHITENULL_llll);
		create_simple_notation_item_undo_tick((t_notation_obj *) x, get_activeitem_undo_item((t_notation_obj *) x), k_UNDO_MODIFICATION_CHANGE);
		change_notation_item_slot_value((t_notation_obj *) x, x->r_ob.active_slot_notationitem, x->r_ob.active_slot_num, 1, new_text_as_llll);
		llll_free(new_text_as_llll);
        if (x->r_ob.link_lyrics_to_slot > 0 && x->r_ob.link_lyrics_to_slot - 1 == x->r_ob.active_slot_num) {
            t_chord *ch = notation_item_chord_get_parent((t_notation_obj *)x, x->r_ob.active_slot_notationitem);
			if (ch)
                ch->need_recompute_parameters = true;
        }
		handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_SLOT);
    } else if (x->r_ob.is_editing_type == k_DYNAMICS_IN_SLOT) {
        t_llll *new_text_as_llll = llll_get();
        llll_appendsym(new_text_as_llll, gensym(text), 0, WHITENULL_llll);
        create_simple_notation_item_undo_tick((t_notation_obj *) x, get_activeitem_undo_item((t_notation_obj *) x), k_UNDO_MODIFICATION_CHANGE);
        change_notation_item_slot_value((t_notation_obj *) x, x->r_ob.active_slot_notationitem, x->r_ob.active_slot_num, 1, new_text_as_llll);
        llll_free(new_text_as_llll);
        if (x->r_ob.link_dynamics_to_slot > 0 && x->r_ob.link_dynamics_to_slot - 1 == x->r_ob.active_slot_num) {
            t_chord *ch = notation_item_chord_get_parent((t_notation_obj *)x, x->r_ob.active_slot_notationitem);
            if (ch)
                ch->need_recompute_parameters = true;
        }
        handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_SLOT);
	} else if (x->r_ob.is_editing_type == k_LYRICS && x->r_ob.is_editing_chord && x->r_ob.is_editing_chord->firstnote) {
		t_llll *new_text_as_llll = llll_get();
		llll_appendsym(new_text_as_llll, gensym(text), 0, WHITENULL_llll);
		create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)x->r_ob.is_editing_chord, k_UNDO_MODIFICATION_CHANGE);
		change_note_slot_value((t_notation_obj *) x, x->r_ob.is_editing_chord->firstnote, x->r_ob.link_lyrics_to_slot - 1, 1, new_text_as_llll);
		llll_free(new_text_as_llll);
		x->r_ob.is_editing_chord->need_recompute_parameters = true;
		handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_LYRICS);
        
    } else if (x->r_ob.is_editing_type == k_DYNAMICS && x->r_ob.is_editing_chord) {
        t_notation_item *nitem = notation_item_get_to_which_dynamics_should_be_assigned((t_notation_obj *)x, (t_notation_item *)x->r_ob.is_editing_chord);
        if (nitem) {
            t_llll *new_text_as_llll = llll_get();
            llll_appendsym(new_text_as_llll, gensym(text), 0, WHITENULL_llll);
            create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)x->r_ob.is_editing_chord, k_UNDO_MODIFICATION_CHANGE);
            change_notation_item_slot_value((t_notation_obj *) x, nitem, x->r_ob.link_dynamics_to_slot - 1, 1, new_text_as_llll);
            llll_free(new_text_as_llll);
            x->r_ob.is_editing_chord->need_recompute_parameters = true;
            handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_DYNAMICS);
        }
    } else if (x->r_ob.is_editing_type == k_LLLL_IN_SLOT) {
		t_llll *my_llll = llll_from_text_buf(text, false);
		if (my_llll) {
			create_simple_notation_item_undo_tick((t_notation_obj *) x, get_activeitem_undo_item((t_notation_obj *) x), k_UNDO_MODIFICATION_CHANGE);
#ifdef BACH_NEW_LLLLSLOT_SYNTAX
            // nothing to do
#else
			llll_wrap_once(&my_llll);
#endif
			change_notation_item_slot_value((t_notation_obj *) x, x->r_ob.active_slot_notationitem, x->r_ob.active_slot_num, 1, my_llll);
			llll_free(my_llll);
			handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_SLOT);
		}
    } else if (x->r_ob.is_editing_type == k_NUMBER_IN_SLOT) {
        t_llll *ll = llll_from_text_buf(text, false);
        create_simple_notation_item_undo_tick((t_notation_obj *) x, get_activeitem_undo_item((t_notation_obj *) x), k_UNDO_MODIFICATION_CHANGE);
        change_notation_item_slot_value((t_notation_obj *) x, x->r_ob.active_slot_notationitem, x->r_ob.active_slot_num, 1, ll);
        llll_free(ll);
        handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_SLOT);
	}
	end_editing_textfield((t_notation_obj *) x);
}



void roll_copy_selection(t_roll *x, char cut)
{
	if (clipboard.gathered_syntax)
		llll_free(clipboard.gathered_syntax); 
    if (clipboard.markers)
        llll_free(clipboard.markers);
	
	clipboard.gathered_syntax = get_selection_gathered_syntax(x);
    clipboard.markers = get_markers_as_llll((t_notation_obj *)x, 2, 0, 0, false, k_CONSIDER_FOR_SAVING, 0);
	clipboard.onset = get_selection_leftmost_onset(x);
	
	clipboard.type = k_SELECTION_CONTENT;
	clipboard.object = k_NOTATION_OBJECT_ROLL;
	if (cut) {
		roll_delete_selection(x, false);
		handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CUT); 
	}
}

void roll_paste_clipboard(t_roll *x, char keep_original_onsets, double force_onset, char keep_original_voice, long force_first_voice, char snap, char also_clear_selection)
{
    force_first_voice += 1; // turning it into 1-based
    
	t_llll *roll_to_paste = llll_clone(clipboard.gathered_syntax);
    t_llll *markers_to_paste = llll_clone(clipboard.markers);
	
//	dev_llll_print(roll_to_paste, NULL, 0, 0, NULL);
	
	lock_general_mutex((t_notation_obj *)x);
    if (also_clear_selection) {
        clear_preselection((t_notation_obj *) x);
        clear_selection((t_notation_obj *) x);
    }
	x->must_append_chords = true;
	x->pasting_chords = true;
	x->must_preselect_appended_chords = true;

	if (keep_original_onsets) // shift the selection to the mouse position
		x->must_apply_delta_onset = 0.;
	else
		x->must_apply_delta_onset = force_onset - clipboard.onset;
	
	if (!keep_original_voice) {
		// deleting empty lists at the beginning of the roll_to_paste: we need the first actual content to be exactly at the voice under the mouse position

        // unless markers are copied: in which case we assume that the we keep original voices!!!
        if (!markers_to_paste || markers_to_paste->l_size <= 1) { // just the "markers" symbol
            
            while (roll_to_paste && roll_to_paste->l_head &&
                   hatom_gettype(&roll_to_paste->l_head->l_hatom) == H_LLLL && hatom_getllll(&roll_to_paste->l_head->l_hatom)->l_size == 0)
                llll_behead(roll_to_paste);
            
            // shift the clipboard voice to the mouse position
            long v, voice_number = force_first_voice;
            for (v = 1; v < voice_number; v++)
                llll_prependllll(roll_to_paste, llll_get(), 0, WHITENULL_llll);
        }
	}
	llll_free(llll_slice(roll_to_paste, x->r_ob.num_voices));
	unlock_general_mutex((t_notation_obj *)x);	
    
    // rescale marker onsets
    t_llllelem *elem;
    for (elem = markers_to_paste->l_head; elem; elem = elem->l_next) {
        if (hatom_gettype(&elem->l_hatom) == H_LLLL) {
            t_llll *ll = hatom_getllll(&elem->l_hatom);
            if (ll && ll->l_head && is_hatom_number(&ll->l_head->l_hatom))
                hatom_setdouble(&ll->l_head->l_hatom, hatom_getdouble(&ll->l_head->l_hatom) + x->must_apply_delta_onset);
        }
    }
    create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
    set_markers_from_llll((t_notation_obj *)x, markers_to_paste, true, true);
    
	set_roll_from_llll(x, roll_to_paste, true);	// <<< with must_append_chords = true the undo ticks are automatically added inside here
	x->pasting_chords = false;
	llll_free(roll_to_paste);
    llll_free(markers_to_paste);
	
	lock_general_mutex((t_notation_obj *)x);	
	move_preselecteditems_to_selection((t_notation_obj *) x, k_SELECTION_MODE_INVOLUTIVE, false, false);
	unlock_general_mutex((t_notation_obj *)x);
	
    if (snap)
        snap_onset_tail_pitch_to_grid_for_selection_if_needed(x);

    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_PASTE);
}



/////////////////////////////////////
////////// LINEAR EDITING STUFF /////
/////////////////////////////////////

t_chord *roll_get_operating_chord_for_linear_edit(t_roll *x) {
    if (!x->r_ob.notation_cursor.voice)
        return NULL;
    return x->r_ob.notation_cursor.chord;
}

t_chord *roll_make_chord_or_note_sharp_or_flat_on_linear_edit(t_roll *x, char direction)
{
    double step = (200/x->r_ob.tone_division);
    t_rational step_acc = genrat(1, x->r_ob.tone_division);
    t_chord *chord = roll_get_operating_chord_for_linear_edit(x);
    
    if (chord) {
        t_note *nt, *cursor_nt = NULL;
        
        create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)chord, k_UNDO_MODIFICATION_CHANGE);
        
        for (nt = chord->firstnote; nt; nt = nt->next) {
            if (note_get_screen_midicents(nt) == x->r_ob.notation_cursor.midicents) {
                cursor_nt = nt;
                break;
            }
        }
        
        for (nt = chord->firstnote; nt; nt = nt->next) {
            if (!cursor_nt || cursor_nt == nt) {
                nt->midicents = nt->midicents + step * direction;
                constraint_midicents_depending_on_editing_ranges((t_notation_obj *)x, &nt->midicents, chord->voiceparent->v_ob.number);

                note_set_user_enharmonicity(nt, t_pitch(nt->pitch_displayed.degree(), rat_rat_sum(nt->pitch_displayed.alter(), rat_long_prod(step_acc, direction)), nt->pitch_displayed.octave()));
                
                calculate_chord_parameters((t_notation_obj *) x, nt->parent, get_voice_clef((t_notation_obj *)x, (t_voice *)nt->parent->voiceparent), true);
            }
        }
    }
    return chord;
}


void roll_exit_linear_edit(t_roll *x)
{
//    if (x->r_ob.notation_cursor.voice)
//        end_editing_measure_in_linear_edit(x, x->r_ob.notation_cursor.measure);
    
    x->r_ob.num_speedy_tuplets = 0;
    x->r_ob.notation_cursor.voice = NULL;	// This means: NO cursor
    x->r_ob.notation_cursor.measure = NULL;
    x->r_ob.notation_cursor.chord = NULL;
    x->r_ob.notation_cursor.onset = 0;
    x->r_ob.notation_cursor.midicents = 6000;
    x->r_ob.notation_cursor.step = 0;
    x->r_ob.is_linear_editing = false;
    update_all_accidentals_if_needed(x);
    recompute_total_length((t_notation_obj *) x);
    invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
}

t_chord *roll_find_chord_at_onset(t_roll *x, t_voice *voice, double onset, double tolerance)
{
    t_chord *temp;
    for (temp = get_first_chord((t_notation_obj *)x, voice); temp; temp = temp->next)
        if (temp->onset >= onset - tolerance && temp->onset <= onset + tolerance)
            return temp;
    return NULL;
}

void roll_force_inscreen_ms_rolling_while_editing(t_roll *x)
{
    if (!x->r_ob.notation_cursor.voice)
        return;
    
    if (x->r_ob.notation_cursor.onset < x->r_ob.screen_ms_start || x->r_ob.notation_cursor.onset > x->r_ob.screen_ms_end)
    force_inscreenpos_ms(x, 0.2, x->r_ob.notation_cursor.onset, true, false, false);
}

t_chord *roll_change_pitch_from_linear_edit(t_roll *x, long diatonic_step)
{
    t_chord *chord = roll_get_operating_chord_for_linear_edit(x);
    long oct = x->r_ob.notation_cursor.midicents / 1200;
    long mc = diatonicstep2midicents(diatonic_step, oct);
    if (labs((mc + 1200) - x->r_ob.notation_cursor.midicents) < labs(mc - x->r_ob.notation_cursor.midicents))
        mc += 1200;
    else if (labs((mc - 1200) - x->r_ob.notation_cursor.midicents) < labs(mc - x->r_ob.notation_cursor.midicents))
        mc -= 1200;
    
    double mc_double = mc;
    constraint_midicents_depending_on_editing_ranges((t_notation_obj *)x, &mc_double, chord->voiceparent->v_ob.number);
    mc = round(mc_double);
    
    if (chord) {
        t_note *nt, *cursor_nt = NULL;
        create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)chord, k_UNDO_MODIFICATION_CHANGE);
        
        for (nt = chord->firstnote; nt; nt = nt->next) {
            if (note_get_screen_midicents(nt) == x->r_ob.notation_cursor.midicents) {
                cursor_nt = nt;
                break;
            }
        }
        
        
        for (nt = chord->firstnote; nt; nt = nt->next) {
            if (!cursor_nt || cursor_nt == nt) {
                nt->midicents = mc;
                nt->pitch_displayed.set(nt->pitch_displayed.degree(), long2rat(0), nt->pitch_displayed.octave());
                note_set_auto_enharmonicity(nt);
                note_compute_approximation((t_notation_obj *)x, nt);
                calculate_chord_parameters((t_notation_obj *) x, nt->parent, get_voice_clef((t_notation_obj *)x, (t_voice *)nt->parent->voiceparent), true);
            }
        }
        
    }
    return chord;
}


void roll_linear_edit_move_onset_ms(t_roll *x, double ms, char also_round)
{
    x->r_ob.notation_cursor.onset += ms;
    if (also_round)
       x->r_ob.notation_cursor.onset = round(x->r_ob.notation_cursor.onset);
    
    if (x->r_ob.notation_cursor.onset < 0)
        x->r_ob.notation_cursor.onset = 0;
    
    x->r_ob.notation_cursor.chord = roll_find_chord_at_onset(x, x->r_ob.notation_cursor.voice, x->r_ob.notation_cursor.onset, 0);
}


void roll_linear_edit_move_onset(t_roll *x, long num_steps, char faster, char to_chord_tail_if_chord_is_edited)
{
    if (to_chord_tail_if_chord_is_edited && x->r_ob.notation_cursor.chord) {
        x->r_ob.notation_cursor.onset = x->r_ob.notation_cursor.chord->onset + chord_get_max_duration((t_notation_obj *)x, x->r_ob.notation_cursor.chord);
    } else if (x->r_ob.show_grid && x->r_ob.snap_linear_edit_to_grid_when_editing) {
        x->r_ob.notation_cursor.onset += num_steps * x->r_ob.grid_step_ms/(faster ? 1 : x->r_ob.grid_subdivisions);
    } else {
        double step = num_steps * x->r_ob.linear_edit_time_step;
        x->r_ob.notation_cursor.onset += step * (faster ? 4 : 1);
    }
    
    if (x->r_ob.notation_cursor.onset < 0)
        x->r_ob.notation_cursor.onset = 0;
    
    x->r_ob.notation_cursor.chord = roll_find_chord_at_onset(x, x->r_ob.notation_cursor.voice, x->r_ob.notation_cursor.onset, 1);
}

void roll_linear_edit_snap_to_chord(t_roll *x)
{
    x->r_ob.notation_cursor.chord = roll_find_chord_at_onset(x, x->r_ob.notation_cursor.voice, x->r_ob.notation_cursor.onset, 0);
}

void roll_linear_edit_snap_cursor_to_grid(t_roll *x)
{
    double position_ms = x->r_ob.notation_cursor.onset;
    position_ms = x->r_ob.current_first_grid_ms + x->r_ob.current_grid_subdivision_ms * round((position_ms - x->r_ob.current_first_grid_ms)/x->r_ob.current_grid_subdivision_ms);
    if (position_ms < 0)
        position_ms = 0;
    x->r_ob.notation_cursor.onset = position_ms;
    roll_linear_edit_snap_to_chord(x);
}


double roll_linear_edit_get_duration(t_roll *x, long number)
{
    if (x->r_ob.show_grid && x->r_ob.snap_linear_edit_to_grid_when_editing)
        return number * x->r_ob.grid_step_ms/x->r_ob.grid_subdivisions;

    number = positive_mod(number - 1, 10);
    return x->r_ob.linear_edit_time_multipliers[number] * x->r_ob.linear_edit_time_step;
}

// if force_diatonic_step == -1, the note is added automatically depending on the notation cursor y position
void roll_add_note_to_chord_from_linear_edit(t_roll *x, long number, long force_diatonic_step, char add_undo_tick)
{
    t_note *this_nt;
    double argv[2];
    
    if (x->r_ob.notation_cursor.chord) {
        argv[0] = x->r_ob.notation_cursor.chord->num_notes >= 1 ? x->r_ob.notation_cursor.chord->firstnote->duration :roll_linear_edit_get_duration(x, number);
        if (force_diatonic_step >= 0) {
            long oct = x->r_ob.notation_cursor.midicents / 1200;
            long mc = diatonicstep2midicents(force_diatonic_step, oct);
            if (labs((mc + 1200) - x->r_ob.notation_cursor.midicents) < labs(mc - x->r_ob.notation_cursor.midicents))
                mc += 1200;
            else if (labs((mc - 1200) - x->r_ob.notation_cursor.midicents) < labs(mc - x->r_ob.notation_cursor.midicents))
                mc -= 1200;
            argv[1] = mc;
        } else
            argv[1] = x->r_ob.notation_cursor.midicents;
        
        constraint_midicents_depending_on_editing_ranges((t_notation_obj *)x, &(argv[1]), x->r_ob.notation_cursor.chord->voiceparent->v_ob.number);
        
        this_nt = build_note_from_ac_av((t_notation_obj *) x, 2, argv);
        if (this_nt)
            set_slots_values_to_note_from_llll((t_notation_obj *)x, this_nt, x->r_ob.default_noteslots);
        
        if (add_undo_tick)
            create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)x->r_ob.notation_cursor.chord, k_UNDO_MODIFICATION_CHANGE);
        
        insert_note((t_notation_obj *) x, x->r_ob.notation_cursor.chord, this_nt, 0);
        note_compute_approximation((t_notation_obj *) x, this_nt);
        calculate_chord_parameters((t_notation_obj *) x, x->r_ob.notation_cursor.chord, get_voice_clef((t_notation_obj *)x, (t_voice *)x->r_ob.notation_cursor.chord->voiceparent), false);
    }
}




t_chord *roll_add_new_chord_from_linear_edit(t_roll *x, char number, long force_diatonic_step)
{
    if (x->r_ob.notation_cursor.voice) {
        t_chord *new_chord =addchord_from_notes(x, x->r_ob.notation_cursor.voice->number, x->r_ob.notation_cursor.onset, 0, 0, NULL, NULL, false, 0);
        x->r_ob.notation_cursor.chord = new_chord;
        roll_add_note_to_chord_from_linear_edit(x, number, force_diatonic_step, false);
        calculate_chord_parameters((t_notation_obj *) x, new_chord, get_voice_clef((t_notation_obj *)x, (t_voice *)new_chord->voiceparent), true);
        return new_chord;
    }
    return NULL;
}



char delete_selected_lyrics_and_dynamics(t_roll *x, char delete_lyrics, char delete_dynamics)
{
    //delete chords/notes within the selected region: actually, it turns them into rests
    t_notation_item *curr_it, *next_item = NULL;
    t_notation_item *lambda_it = x->r_ob.lambda_selected_item_ID > 0 ? (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, x->r_ob.lambda_selected_item_ID) : NULL;
    
    char changed = 0;
    lock_general_mutex((t_notation_obj *)x);
    for (curr_it = x->r_ob.firstselecteditem; curr_it; curr_it = next_item) { // cycle on the selected items
        
        if (lambda_it && (lambda_it == curr_it || // lambda item is exactly the item we're deleting..
                          notation_item_is_ancestor_of((t_notation_obj *)x, lambda_it, curr_it) || // or one of its ancestors...
                          notation_item_is_ancestor_of((t_notation_obj *)x, curr_it, lambda_it))) { // or one of its progeny...
            //			cpost("Trying to delete item %p (type %ld). Can't.", curr_it, curr_it->type);
            object_error((t_object *)x, "Can't delete item, it's being output from the playout!");
            continue;
        }
        
        next_item = curr_it->next_selected;

        if (curr_it->type == k_LYRICS && delete_lyrics) {
            t_lyrics *ly = (t_lyrics *)curr_it;
            create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)ly->owner, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
            changed |= delete_chord_lyrics((t_notation_obj *) x, (t_chord *) ly->owner);
            
        } else if (curr_it->type == k_DYNAMICS && delete_dynamics) {
            t_dynamics *dy = (t_dynamics *)curr_it;
            create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)dy->owner, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
            changed |= delete_chord_dynamics((t_notation_obj *) x, (t_chord *) dy->owner);
        }
        curr_it = next_item;
    }
    unlock_general_mutex((t_notation_obj *)x);	
    
    check_correct_scheduling((t_notation_obj *)x, true);
    
    return changed;
}



char roll_key_linearedit(t_roll *x, t_object *patcherview, long keycode, long modifiers, long textcharacter)
{
    if (keycode == '+') {
        if (modifiers & eControlKey) {
            if (x->r_ob.notation_cursor.chord) {
                t_note *note;
                for (note = x->r_ob.notation_cursor.chord->firstnote; note; note = note->next) {
                    if (modifiers & eAltKey)
                        note->duration *= 2.;
                    else
                        note->duration = round(note->duration + 1 * (modifiers & eShiftKey ? 10 : 1));
                }
                recompute_total_length((t_notation_obj *)x);
                invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
            } else {
                if (modifiers & eAltKey)
                    x->r_ob.linear_edit_time_step *= 2.;
                else
                    x->r_ob.linear_edit_time_step = round(x->r_ob.linear_edit_time_step + 1);
                x->r_ob.linear_edit_time_step = MAX(1, x->r_ob.linear_edit_time_step);
                notationobj_redraw((t_notation_obj *)x);
            }
        } else {
            t_chord *ch = roll_make_chord_or_note_sharp_or_flat_on_linear_edit(x, 1);	// edited chord
            if (ch)
                ch->need_recompute_parameters = true;
            
            handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LINEAR_EDIT_ADD_SHARP);
            
            if (x->r_ob.playback_during_linear_editing && ch)
                send_chord_as_llll((t_notation_obj *) x, ch, 6, k_CONSIDER_FOR_DUMPING, -1);
        }
        return 1;
    } else if (keycode == '-') {
        if (modifiers & eControlKey) {
            if (x->r_ob.notation_cursor.chord) {
                t_note *note;
                for (note = x->r_ob.notation_cursor.chord->firstnote; note; note = note->next) {
                    if (modifiers & eAltKey)
                        note->duration /= 2.;
                    else
                        note->duration = round(note->duration - 1 * (modifiers & eShiftKey ? 10 : 1));
                    note->duration = MAX(note->duration, 0);
                }
                recompute_total_length((t_notation_obj *)x);
                invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
            } else {
                if (modifiers & eAltKey)
                    x->r_ob.linear_edit_time_step /= 2.;
                else
                    x->r_ob.linear_edit_time_step = round(x->r_ob.linear_edit_time_step - 1);
                x->r_ob.linear_edit_time_step = MAX(1, x->r_ob.linear_edit_time_step);
                notationobj_redraw((t_notation_obj *)x);
            }
        } else {
            t_chord *ch = roll_make_chord_or_note_sharp_or_flat_on_linear_edit(x, -1);	// edited chord
            if (ch)
                ch->need_recompute_parameters = true;
            
            handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LINEAR_EDIT_ADD_FLAT);
            
            if (x->r_ob.playback_during_linear_editing && ch)
                send_chord_as_llll((t_notation_obj *) x, ch, 6, k_CONSIDER_FOR_DUMPING, -1);
        }
        return 1;
    
    } else {
        switch (keycode) {
            case JKEY_ESC:
                if (modifiers & eAltKey) {
                    invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
                } else {
                    roll_exit_linear_edit(x);
                    invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
                }
                return 1;
                break;
            case JKEY_LEFTARROW:
            {
                if (modifiers & eCommandKey) {
                    t_chord *new_ch = x->r_ob.notation_cursor.chord ? get_prev_chord(x->r_ob.notation_cursor.chord) : get_first_chord_before_ms((t_notation_obj *)x, x->r_ob.notation_cursor.voice, x->r_ob.notation_cursor.onset);
                    if (new_ch) {
                        double new_ch_tail = new_ch->onset + chord_get_max_duration((t_notation_obj *)x, new_ch);
                        if (new_ch && new_ch_tail > x->r_ob.notation_cursor.onset - 1) {
                            // snap to chord
                            x->r_ob.notation_cursor.chord = new_ch;
                            x->r_ob.notation_cursor.onset = new_ch->onset;
                        } else {
                            // snap to tail
                            x->r_ob.notation_cursor.chord = NULL;
                            x->r_ob.notation_cursor.onset = new_ch_tail;
                        }
                    }
                } else if (modifiers & eAltKey) {
                    roll_linear_edit_move_onset_ms(x, -1 * (modifiers & eShiftKey ? 10 : 1), true);
                } else {
                    roll_linear_edit_move_onset(x, -1, modifiers & eShiftKey, false);
                }
                
                roll_force_inscreen_ms_rolling_while_editing(x);
                
                invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
                return 1;
                break;
            }
            case JKEY_RIGHTARROW:
            {
                if (modifiers & eCommandKey) {
                    t_chord *new_ch = x->r_ob.notation_cursor.chord ? get_next_chord(x->r_ob.notation_cursor.chord) : get_first_chord_after_ms((t_notation_obj *)x, x->r_ob.notation_cursor.voice, x->r_ob.notation_cursor.onset);
                    double old_ch_tail = x->r_ob.notation_cursor.chord ? x->r_ob.notation_cursor.chord->onset + chord_get_max_duration((t_notation_obj *)x, x->r_ob.notation_cursor.chord) : 0;
                    if (new_ch && (!x->r_ob.notation_cursor.chord || old_ch_tail > new_ch->onset - 1)) {
                        // snap to chord
                        x->r_ob.notation_cursor.chord = new_ch;
                        x->r_ob.notation_cursor.onset = new_ch->onset;
                    } else if (x->r_ob.notation_cursor.chord){
                        // snap to tail
                        x->r_ob.notation_cursor.chord = NULL;
                        x->r_ob.notation_cursor.onset = old_ch_tail;
                    }
                } else if (modifiers & eAltKey) {
                    roll_linear_edit_move_onset_ms(x, 1 * (modifiers & eShiftKey ? 10 : 1), true);
                } else {
                    roll_linear_edit_move_onset(x, 1, modifiers & eShiftKey, false);
                }
                
                roll_force_inscreen_ms_rolling_while_editing(x);
                
                invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
                return 1;
                break;
            }
            case JKEY_UPARROW:
            {
                if (modifiers & eCommandKey) {
                    t_voice *prev = get_prev_voice((t_notation_obj *)x, x->r_ob.notation_cursor.voice);
                    if (prev) {
                        x->r_ob.notation_cursor.voice = prev;
                        x->r_ob.notation_cursor.chord = roll_find_chord_at_onset(x, x->r_ob.notation_cursor.voice, x->r_ob.notation_cursor.onset, 0);
                    }
                } else {
                    move_linear_edit_cursor_depending_on_edit_ranges((t_notation_obj *)x, modifiers & eShiftKey ? 7 : 1, modifiers);
                    x->r_ob.notation_cursor.midicents = scaleposition_to_midicents(x->r_ob.notation_cursor.step);
                }
                invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
                return 1;
                break;
            }
            case JKEY_DOWNARROW:
            {
                if (modifiers & eCommandKey) {
                    t_voice *next = x->r_ob.notation_cursor.voice && x->r_ob.notation_cursor.voice->number >= x->r_ob.num_voices - 1 ? NULL : get_next_voice((t_notation_obj *)x, x->r_ob.notation_cursor.voice);
                    if (next) {
                        x->r_ob.notation_cursor.voice = next;
                        x->r_ob.notation_cursor.chord = roll_find_chord_at_onset(x, x->r_ob.notation_cursor.voice, x->r_ob.notation_cursor.onset, 0);
                    }
                } else {
                    move_linear_edit_cursor_depending_on_edit_ranges((t_notation_obj *)x, modifiers & eShiftKey ? -7 : -1, modifiers);
                    x->r_ob.notation_cursor.midicents = scaleposition_to_midicents(x->r_ob.notation_cursor.step);
                }
                invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
                return 1;
                break;
            }

            case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g':
            {
                if (x->r_ob.notation_cursor.chord) {
                    // change pitch of active chord
                    
                    create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)x->r_ob.notation_cursor.chord, k_UNDO_MODIFICATION_CHANGE);
                    
                    roll_change_pitch_from_linear_edit(x, ((keycode - 'a') + 5) % 7);
                    
                    if (x->r_ob.playback_during_linear_editing && x->r_ob.notation_cursor.chord)
                        send_chord_as_llll((t_notation_obj *) x, x->r_ob.notation_cursor.chord, 6, k_CONSIDER_FOR_DUMPING, -1);
                    
                    handle_change((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LINEAR_EDIT_CHANGE_PITCH);
                    invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
                } else {
                    // add new chord
                    x->r_ob.force_diatonic_step = ((keycode - 'a') + 5) % 7;
                    // we add a new note with the given forced diatonic step by calling score_key and "pretending" the user has clicked on a
                    // numeric key (determining the note duration).
                    roll_key(x, patcherview, x->r_ob.linear_edit_last_inserted_dur, modifiers, textcharacter);
                    return 1;
                }
                return 1;
                break;
            }
            case 'n': // add new note to chord
            {
                if (x->r_ob.notation_cursor.chord) {
                    roll_add_note_to_chord_from_linear_edit(x, 4, -1, true);
                    if (x->r_ob.auto_jump_to_next_chord)
                        roll_linear_edit_move_onset(x, 1, false, true);
                    roll_force_inscreen_ms_rolling_while_editing(x);
                    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LINEAR_EDIT_ADD_NOTE);
                    if (x->r_ob.playback_during_linear_editing && x->r_ob.notation_cursor.chord)
                        send_chord_as_llll((t_notation_obj *) x, x->r_ob.notation_cursor.chord, 6, k_CONSIDER_FOR_DUMPING, -1);
                }
                return 1;
                break;
            }
                
            case JKEY_DELETE:
            case JKEY_BACKSPACE: // delete chord or note
            {
                t_chord *ch = roll_get_operating_chord_for_linear_edit(x);
                if (ch) {
                    if (modifiers & eShiftKey){
                        t_note *nt;
                        for (nt = ch->firstnote; nt; nt = nt->next) {
                            if (note_get_screen_midicents(nt) == x->r_ob.notation_cursor.midicents) {
                                char num_notes = ch->num_notes;
                                if (num_notes == 1) { // delete chord
                                    create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_UNDO_MODIFICATION_ADD);
                                    delete_chord_from_voice((t_notation_obj *) x, ch, ch->prev, false);
                                    x->r_ob.notation_cursor.chord = NULL;
                                    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LINEAR_EDIT_DELETE_CHORD);
                                } else  {
                                    create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)ch, k_UNDO_MODIFICATION_CHANGE);
                                    note_delete((t_notation_obj *)x, nt, false);
                                    ch->need_recompute_parameters = true;
                                    
                                    if (x->r_ob.playback_during_linear_editing && ch)
                                        send_chord_as_llll((t_notation_obj *) x, ch, 6, k_CONSIDER_FOR_DUMPING, -1);
                                    
                                    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LINEAR_EDIT_DELETE_NOTE);
                                }
                                break;
                            }
                        }
                    } else {
                        create_simple_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_UNDO_MODIFICATION_ADD);
                        delete_chord_from_voice((t_notation_obj *) x, ch, ch->prev, false);
                        x->r_ob.notation_cursor.chord = NULL;
                        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LINEAR_EDIT_DELETE_CHORD);
                    }
                }
                return 1;
                break;
            }
                
//            case 46: // no dots in bach.roll
            case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57:	// 46 is the dot
            case JKEY_NUMPAD0: case JKEY_NUMPAD1: case JKEY_NUMPAD2: case JKEY_NUMPAD3: case JKEY_NUMPAD4: case JKEY_NUMPAD5: case JKEY_NUMPAD6: case JKEY_NUMPAD7: case JKEY_NUMPAD8: case JKEY_NUMPAD9:
                
                // numbers
                if (keycode == 46 && modifiers & eShiftKey)
                    break;
                
                if (keycode == JKEY_NUMPAD0) keycode = 48;
                else if (keycode == JKEY_NUMPAD1) keycode = 49;
                else if (keycode == JKEY_NUMPAD2) keycode = 50;
                else if (keycode == JKEY_NUMPAD3) keycode = 51;
                else if (keycode == JKEY_NUMPAD4) keycode = 52;
                else if (keycode == JKEY_NUMPAD5) keycode = 53;
                else if (keycode == JKEY_NUMPAD6) keycode = 54;
                else if (keycode == JKEY_NUMPAD7) keycode = 55;
                else if (keycode == JKEY_NUMPAD8) keycode = 56;
                else if (keycode == JKEY_NUMPAD9) keycode = 57;
                
                x->r_ob.linear_edit_last_inserted_dur = keycode;
                
                if (modifiers & eCommandKey && keycode > 49) {
                    // Splitting?
/*                    if (x->r_ob.notation_cursor.chord) {
                        create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)x->r_ob.notation_cursor.measure, k_UNDO_MODIFICATION_CHANGE);
                        lock_general_mutex((t_notation_obj *)x);
                        split_chord(x, x->r_ob.notation_cursor.chord, keycode - 48, x->r_ob.notation_cursor.chord->parent->lock_rhythmic_tree || x->r_ob.tree_handling == k_RHYTHMIC_TREE_HANDLING_TAKE_FOR_GRANTED);
                        perform_analysis_and_change(x, NULL, NULL, x->r_ob.tree_handling == k_RHYTHMIC_TREE_HANDLING_IGNORE ? k_BEAMING_CALCULATION_FROM_SCRATCH : k_BEAMING_CALCULATION_DONT_CHANGE_LEVELS);
                        unlock_general_mutex((t_notation_obj *)x);
                        x->r_ob.need_recompute_chords_double_onset = true;
                        set_need_perform_analysis_and_change_flag((t_notation_obj *)x);
                        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LINEAR_EDIT_SPLIT_CHORD);
                    }  */
                } else {
                    if (keycode > 48 && keycode < 57) {
                        e_undo_operations op = k_UNDO_OP_LINEAR_EDIT_ADD_CHORD;
                        t_chord *edited_chord = NULL;

                        if (x->r_ob.notation_cursor.chord) {
                            // there's already a chord, we change its duration
                            create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)x->r_ob.notation_cursor.chord, k_UNDO_MODIFICATION_CHANGE);
                            t_note *note;
                            for (note = x->r_ob.notation_cursor.chord->firstnote; note; note = note->next)
                                note->duration = roll_linear_edit_get_duration(x, keycode - 48);
                            op = k_UNDO_OP_LINEAR_EDIT_CHANGE_CHORD_DURATION;
                            edited_chord = x->r_ob.notation_cursor.chord;
                            
                        } else if (x->r_ob.notation_cursor.voice) {

                            edited_chord = roll_add_new_chord_from_linear_edit(x, keycode - 48, x->r_ob.force_diatonic_step);
                            x->r_ob.force_diatonic_step = -1;

                            create_simple_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)edited_chord, k_UNDO_MODIFICATION_DELETE);
                            
                        }

                        if (x->r_ob.playback_during_linear_editing && edited_chord && keycode != 46)
                            send_chord_as_llll((t_notation_obj *) x, edited_chord, 6, k_CONSIDER_FOR_DUMPING, -1);
                        
                        if (x->r_ob.auto_jump_to_next_chord)
                            roll_linear_edit_move_onset(x, 1, false, true);
                        
                        recompute_total_length((t_notation_obj *)x);
                        roll_force_inscreen_ms_rolling_while_editing(x);
                        
                        handle_change((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, op);
                    }
                }
                return 1;
                break;
            default:
                break;
        }
    }
    
    return 0;
}


long roll_key(t_roll *x, t_object *patcherview, long keycode, long modifiers, long textcharacter)
{
	int j;
	
	llll_format_modifiers(&modifiers, &keycode);

	if (keycode == 'i' && modifiers == eCommandKey && is_editable((t_notation_obj *)x, k_BACH_INSPECTOR, k_ELEMENT_ACTIONS_NONE)) {
		if (x->r_ob.active_slot_num >= 0 && x->r_ob.active_slot_notationitem) {
			x->non_inspector_ms_screen_start = x->r_ob.screen_ms_start;
			if (x->r_ob.m_inspector.inspector_patcher)
				bring_external_inspector_to_front(&x->r_ob.m_inspector);
			open_bach_inspector((t_notation_obj *)x, &x->r_ob.m_inspector, &x->r_ob.slotinfo[x->r_ob.active_slot_num], k_SLOTINFO);
			invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
			return 1;
            
		} else if (x->r_ob.num_selecteditems == 1 &&
				   (x->r_ob.firstselecteditem->type == k_CHORD || x->r_ob.firstselecteditem->type == k_NOTE ||
					x->r_ob.firstselecteditem->type == k_VOICE || x->r_ob.firstselecteditem->type == k_MARKER ||
					x->r_ob.firstselecteditem->type == k_PITCH_BREAKPOINT)) {
					   if (x->r_ob.m_inspector.inspector_patcher)
						   bring_external_inspector_to_front(&x->r_ob.m_inspector);
					   open_bach_inspector_for_notation_item((t_notation_obj *)x, x->r_ob.firstselecteditem);
					   invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
					   return 1;
				   }
	} else if (keycode == JKEY_RETURN && is_editable((t_notation_obj *)x, k_PLAYCURSOR, k_ELEMENT_ACTIONS_NONE)) {
		if (!x->r_ob.playing) {
			x->r_ob.play_head_start_ms = x->r_ob.show_loop_region && !(modifiers & eShiftKey) ? x->r_ob.loop_region.start.position_ms : 0;
            send_moved_playhead_position((t_notation_obj *) x, 6);
			if (!x->r_ob.show_loop_region || modifiers & eShiftKey) {
				x->r_ob.hscrollbar_pos = 0.;
				redraw_hscrollbar((t_notation_obj *) x, 1);
			} else {
				force_inscreen_ms_to_boundary(x, x->r_ob.play_head_start_ms, false, true, true, false);
				invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
			}
			send_domain(x, 6, NULL);
			return 1;
		}
	}
	
	if (keycode == 'z' && x->r_ob.allow_undo) {
		if (modifiers == eCommandKey) {
			roll_undo(x);
			return 1;
		} else if (modifiers == eCommandKey + eShiftKey) {
			roll_redo(x);
			return 1;
		}
	}
    
    // linear editing?
    if (x->r_ob.notation_cursor.voice && x->r_ob.allow_linear_edit) // linear edit?
        if (roll_key_linearedit(x, patcherview, keycode, modifiers, textcharacter))
            return 1;
  
// THese two lines were in bach.score. No idea of what they refer to.
//        if (!(modifiers & eCommandKey) || (keycode != 'c' && keycode != 'v'))
//            return 1;

	
	// first of all: are we pressing a key associated with some slot?
	if (!(modifiers & eCommandKey) && !(modifiers & eAltKey) && !(modifiers & eControlKey) && is_editable((t_notation_obj *)x, k_SLOT, k_ELEMENT_ACTIONS_NONE)) {
		for (j=0; j<CONST_MAX_SLOTS; j++) {
			if (x->r_ob.slotinfo[j].slot_key == textcharacter) {
				// change slot view
				// detect the selection type
				if (x->r_ob.num_selecteditems == 1 && (x->r_ob.firstselecteditem->type == k_NOTE || x->r_ob.firstselecteditem->type == k_CHORD)) {
					open_slot_window((t_notation_obj *) x, j, notation_item_to_notation_item_for_slot_win_opening((t_notation_obj *)x, x->r_ob.firstselecteditem));
					return 1;
				}
			}
		}
	}
	
	if (keycode == JKEY_TAB && x->r_ob.active_slot_num > -1 && x->r_ob.active_slot_notationitem && is_editable((t_notation_obj *)x, k_SLOT, k_ELEMENT_ACTIONS_NONE)) {
		if (modifiers == eShiftKey){
			// shift+tab on a slot goes to the next one
			open_slot_window((t_notation_obj *) x, MIN(CONST_MAX_SLOTS - 1, x->r_ob.active_slot_num + 1), x->r_ob.active_slot_notationitem);
			return 1;
		} else if (modifiers == (eShiftKey | eAltKey)) {
			// shift+tab on a slot goes to the prev one
			open_slot_window((t_notation_obj *) x, MAX(0, x->r_ob.active_slot_num - 1), x->r_ob.active_slot_notationitem);
			return 1;
		}
	}
	
	if (!(modifiers & eCommandKey) && !(modifiers & eAltKey) && !(modifiers & eControlKey)) {
		for (j=0; j<CONST_MAX_COMMANDS; j++) {
			if (x->r_ob.command_key[j] == textcharacter) {
				// send command values
				selection_send_command(x, modifiers, j, true);
				return 1;
			}
		}
	}
	
	// changing the selected item (e.g. selecting the one above, below, at left, at right...)
	if (modifiers == eCommandKey && is_editable((t_notation_obj *)x, k_SELECTION, k_SINGLE_SELECTION)) {
		if (keycode == JKEY_LEFTARROW) {
			t_notation_item *first_sel_item = get_leftmost_selected_notation_item((t_notation_obj *)x);
			t_notation_item *to_select = first_sel_item ? notation_item_get_at_left((t_notation_obj *)x, first_sel_item, false) : NULL;
			if (to_select) {
				select_single_notation_item_and_force_inscreen((t_notation_obj *)x, to_select);
				return 1;
			}
		} else if (keycode == JKEY_RIGHTARROW) {
			t_notation_item *last_sel_item = get_rightmost_selected_notation_item((t_notation_obj *)x);
			t_notation_item *to_select = last_sel_item ? notation_item_get_at_right((t_notation_obj *)x, last_sel_item, false) : NULL;
			if (to_select) { 
				select_single_notation_item_and_force_inscreen((t_notation_obj *)x, to_select);
				return 1;
			}
		} else if (keycode == JKEY_UPARROW) {
			t_notation_item *first_sel_item = get_leftmost_selected_notation_item((t_notation_obj *)x);
			t_notation_item *to_select = first_sel_item ? notation_item_get_at_top((t_notation_obj *)x, first_sel_item, false) : NULL;
			if (to_select) {
				select_single_notation_item_and_force_inscreen((t_notation_obj *)x, to_select);
				return 1;
			}
		} else if (keycode == JKEY_DOWNARROW) {
			t_notation_item *last_sel_item = get_rightmost_selected_notation_item((t_notation_obj *)x);
			t_notation_item *to_select = last_sel_item ? notation_item_get_at_bottom((t_notation_obj *)x, last_sel_item, false) : NULL;
			if (to_select) {
				select_single_notation_item_and_force_inscreen((t_notation_obj *)x, to_select);
				return 1;
			}
		}
	}
	
	if (x->r_ob.need_hscrollbar && x->r_ob.show_hscrollbar && is_editable((t_notation_obj *)x, k_SCROLLBAR, k_ELEMENT_ACTIONS_NONE)) {
		switch (keycode) { // scrollbar scrolling (with command key)
			case JKEY_LEFTARROW:
				if (modifiers & eCommandKey){ // scroll
					if (modifiers & eShiftKey)
						x->r_ob.hscrollbar_x -= CONST_FASTER_FROM_KEYBOARD * CONST_UX_SCROLLBAR_SHIFT_FROM_KEYBOARD * x->r_ob.zoom_y;
					else
						x->r_ob.hscrollbar_x -= CONST_UX_SCROLLBAR_SHIFT_FROM_KEYBOARD * x->r_ob.zoom_y;
					redraw_hscrollbar((t_notation_obj *)x, 0);
					return 1;
				}
			case JKEY_RIGHTARROW:
				if (modifiers & eCommandKey){ // scroll
					if (modifiers & eShiftKey)
						x->r_ob.hscrollbar_x += CONST_FASTER_FROM_KEYBOARD * CONST_UX_SCROLLBAR_SHIFT_FROM_KEYBOARD * x->r_ob.zoom_y;
					else
						x->r_ob.hscrollbar_x += CONST_UX_SCROLLBAR_SHIFT_FROM_KEYBOARD * x->r_ob.zoom_y;
					redraw_hscrollbar((t_notation_obj *)x, 0);
					return 1;
				}
		}
	} 
	
	if (x->r_ob.selection_type == k_PITCH_BREAKPOINT) { // only bpts selected!
		char there_are_all_tails = only_tails_are_selected((t_notation_obj *) x); // all note tails?

		if (keycode == JKEY_LEFTARROW && there_are_all_tails && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_DURATION)) {
			if (!(modifiers & eCommandKey)) {
				// decrease duration
				if (modifiers & eAltKey) {
					trim_selection_end((t_notation_obj *)x, x->r_ob.snap_tail_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? -x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? -CONST_FASTER_FROM_KEYBOARD : -1.) * CONST_DURATION_SHIFT_FROM_KEYBOARD / x->r_ob.zoom_x)));
					handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_TRIM_END);
				} else {
					change_selection_duration(x, x->r_ob.snap_tail_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? -x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? -CONST_FASTER_FROM_KEYBOARD : -1.) * CONST_DURATION_SHIFT_FROM_KEYBOARD / x->r_ob.zoom_x)));
					handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_DURATION_FOR_SELECTION); 
				}
				return 1;
			}
		} else if (keycode == JKEY_RIGHTARROW && there_are_all_tails && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_DURATION)) {
			if (!(modifiers & eCommandKey)) {
				// increase duration
				if (modifiers & eAltKey) {
					trim_selection_end((t_notation_obj *)x, x->r_ob.snap_tail_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? CONST_FASTER_FROM_KEYBOARD : 1.) * CONST_DURATION_SHIFT_FROM_KEYBOARD  / x->r_ob.zoom_x)));
					handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_TRIM_END);
				} else {
					change_selection_duration(x, x->r_ob.snap_tail_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? CONST_FASTER_FROM_KEYBOARD : 1.) * CONST_DURATION_SHIFT_FROM_KEYBOARD  / x->r_ob.zoom_x)));
					handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_DURATION_FOR_SELECTION); 
				}
				return 1;
			}
		} else if ((keycode == JKEY_UPARROW || keycode == JKEY_DOWNARROW) && is_editable((t_notation_obj *)x, k_PITCH_BREAKPOINT, k_MODIFICATION_PITCH)) {
			change_selection_breakpoint_pitch((t_notation_obj *)x, (keycode == JKEY_UPARROW ? 1 : -1) * (modifiers & eShiftKey ? 1200 : 200 / x->r_ob.tone_division));
			handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, keycode == JKEY_UPARROW ? k_UNDO_OP_SHIFT_PITCH_UP_FOR_SELECTION : k_UNDO_OP_SHIFT_PITCH_DOWN_FOR_SELECTION); 
		} else {
			if ((keycode == JKEY_BACKSPACE || keycode == JKEY_DELETE) && is_editable((t_notation_obj *)x, k_PITCH_BREAKPOINT, k_DELETION)) {
				// delete breakpoints
				reset_selection_tail_gliss((t_notation_obj *) x);
				delete_breakpoints_in_selection((t_notation_obj *) x);
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_PITCH_BREAKPOINTS_IN_SELECTION); 
				return 1;
			} else if (keycode == JKEY_ESC && is_editable((t_notation_obj *)x, k_PITCH_BREAKPOINT, k_MODIFICATION_GENERIC)) {
				// put the slope at 0.
				reset_selection_breakpoint_slope((t_notation_obj *) x); 
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_RESET_PITCH_BREAKPOINTS_SLOPE_FOR_SELECTION); 
				return 1;
			}
		}
    } else if (x->r_ob.selection_type == k_ARTICULATION && (keycode == JKEY_BACKSPACE || keycode == JKEY_DELETE)) { // only measures selected + BACKSPACE
        if (!is_editable((t_notation_obj *)x, k_ARTICULATION, k_DELETION)) return 0;
        delete_articulations_in_selection((t_notation_obj *) x);
        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_ARTICULATIONS_FOR_SELECTION);
        return 1;
    }

	
    if (handle_keys_for_articulations((t_notation_obj *) x, patcherview, keycode, modifiers, textcharacter) && is_editable((t_notation_obj *)x, k_ARTICULATION, k_CREATION)) {
        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_ADD_ARTICULATION_TO_SELECTION);
        return 1;
    }
    
	if (modifiers & eControlKey && keycode >= 'a' && keycode <= 'g' && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_PITCH)) {
		change_pitch_to_selection_from_diatonic_step((t_notation_obj *) x, keycode == 'a' ? 5 : (keycode == 'b' ? 6 : keycode - 'c'));
		handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_PITCH); 
		return 1;
	}
	
	// mixed or notes/chord selection
	switch (keycode) {
		case JKEY_UPARROW:
			// shift note up
			if (!(modifiers & (eCommandKey | eAltKey)) && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_PITCH)) {
				change_pitch_for_selection(x, (!(modifiers & eCommandKey) && (modifiers & eShiftKey)) ? 6 * x->r_ob.tone_division : 1, 0, ((modifiers & eControlKey) && (modifiers & eShiftKey)), true);
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_SHIFT_PITCH_UP_FOR_SELECTION); 
				return 1;
			}
			break;
			
		case JKEY_DOWNARROW:
			// shift note down
			if (!(modifiers & (eCommandKey | eAltKey)) && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_PITCH)) {
				change_pitch_for_selection(x, (!(modifiers & eCommandKey) && (modifiers & eShiftKey)) ? -6 * x->r_ob.tone_division : -1, 0, ((modifiers & eControlKey) && (modifiers & eShiftKey)), true);
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_SHIFT_PITCH_DOWN_FOR_SELECTION); 
				return 1;
			}
			break;
			
		case JKEY_LEFTARROW:
			if ((modifiers & eAltKey) && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_DURATION)){ // change length
				// decrease duration
				if (modifiers & eControlKey) {
					trim_selection_end((t_notation_obj *)x, x->r_ob.snap_tail_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? -x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? -CONST_FASTER_FROM_KEYBOARD : -1.) * CONST_DURATION_SHIFT_FROM_KEYBOARD / x->r_ob.zoom_x)));
					handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_TRIM_END);
				} else {
					change_selection_duration(x, x->r_ob.snap_tail_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? -x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? -CONST_FASTER_FROM_KEYBOARD : -1.) * CONST_DURATION_SHIFT_FROM_KEYBOARD / x->r_ob.zoom_x)));
					handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_DURATION_FOR_SELECTION); 
				}
				return 1;
				break;
			} else if (!(modifiers & eCommandKey) && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET)){
				// shift chord leftwards
				double delta_ms = x->r_ob.snap_onset_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? -x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? -CONST_FASTER_FROM_KEYBOARD : -1.) * CONST_ONSET_SHIFT_FROM_KEYBOARD / x->r_ob.zoom_x));
				change_selection_onset(x, &delta_ms); 
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_ONSET_FOR_SELECTION); 
				return 1;
				break;
			}
			break;
			
		case JKEY_RIGHTARROW:
			if (modifiers & eAltKey) {  // change length
				if (!is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_DURATION))
					return 0;
				// increase duration
				if (modifiers & eControlKey) {
					trim_selection_end((t_notation_obj *)x, x->r_ob.snap_tail_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? CONST_FASTER_FROM_KEYBOARD : 1.) * CONST_DURATION_SHIFT_FROM_KEYBOARD / x->r_ob.zoom_x)));
					handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_TRIM_END);
				} else {
					change_selection_duration(x, x->r_ob.snap_tail_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? CONST_FASTER_FROM_KEYBOARD : 1.) * CONST_DURATION_SHIFT_FROM_KEYBOARD / x->r_ob.zoom_x)));
					handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_DURATION_FOR_SELECTION); 
				}
				return 1;
				break;
			} else if (!(modifiers & eCommandKey)){
				// shift chord rightwards
				if (!is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET))
					return 0;
				
				double delta_ms = x->r_ob.snap_onset_to_grid_when_editing && (x->r_ob.ruler > 0 || x->r_ob.show_grid) ? x->r_ob.current_grid_subdivision_ms * (modifiers & eShiftKey ? x->r_ob.current_num_grid_subdivisions : 1) : (round(((modifiers & eShiftKey) ? CONST_FASTER_FROM_KEYBOARD : 1.) * CONST_ONSET_SHIFT_FROM_KEYBOARD / x->r_ob.zoom_x));
				change_selection_onset(x, &delta_ms);
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CHANGE_ONSET_FOR_SELECTION); 
				return 1;
				break;
			}
			break;
			
        case JKEY_DELETE:
		case JKEY_BACKSPACE:
			// delete notes/chords in selection
            
            delete_selected_lyrics_and_dynamics(x, is_editable((t_notation_obj *)x, k_LYRICS, k_ELEMENT_ACTIONS_NONE), is_editable((t_notation_obj *)x, k_DYNAMICS, k_ELEMENT_ACTIONS_NONE));
            
			if (x->r_ob.active_slot_num >= 0) {
				if (is_editable((t_notation_obj *)x, k_SLOT, k_ELEMENT_ACTIONS_NONE)) {
					if (x->r_ob.selected_slot_items->l_size > 0) {
						create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, get_activeitem_undo_item((t_notation_obj *)x), k_CHORD, k_UNDO_MODIFICATION_CHANGE);
						delete_all_selected_function_points((t_notation_obj *)x, x->r_ob.active_slot_num);
						handle_change_if_there_are_free_undo_ticks((t_notation_obj *)x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_SLOT_CONTENT);
					}
				}
			} else {
				if (is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_DELETION)) {
                    if (modifiers & eShiftKey) {
                        roll_delete_selection_and_transfer_default_slots(x, true);
                        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_RIPPLE_DELETE_SELECTION);
                    } else {
                        roll_delete_selection_and_transfer_default_slots(x, false);
                        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_SELECTION);
                    }
				}
			}
			return 1;
			break;
			
		case JKEY_ESC:
			// return to normal view
            if ((x->r_ob.active_slot_notationitem || x->r_ob.active_slot_num >= 0) && is_editable((t_notation_obj *)x, k_SLOT, k_ELEMENT_ACTIONS_NONE)) {
				close_slot_window((t_notation_obj *)x);
				return 1;
			}
			return 0;
			break;
			
		case 48: case 49: case 50: case 51: case 52: case 53: case 54: case 55: case 56: case 57:
			// change slot view
			// detect the selection type
			if (!(modifiers & eCommandKey) && !(modifiers & eAltKey) && !(modifiers & eControlKey) && (x->r_ob.num_selecteditems == 1) && ((x->r_ob.firstselecteditem->type == k_NOTE) || (x->r_ob.firstselecteditem->type == k_CHORD)) && is_editable((t_notation_obj *)x, k_SLOT, k_ELEMENT_ACTIONS_NONE)) {
				open_slot_window((t_notation_obj *) x,  (keycode == 48) ? 9 : keycode - 49, notation_item_to_notation_item_for_slot_win_opening((t_notation_obj *)x, x->r_ob.firstselecteditem));
				return 1;
			}
			return 0;
			break;
			
		case 'l': // Cmd+L
			if (modifiers & eCommandKey) {
				if (x->r_ob.num_selecteditems == 0) {
					if (modifiers & eShiftKey) {
						x->r_ob.use_loop_region = x->r_ob.use_loop_region ? false : true;
						send_loop_region_on_off((t_notation_obj *)x, 6);
						if (x->r_ob.use_loop_region && x->r_ob.show_loop_region)
							send_loop_region((t_notation_obj *)x, 6);
						if (x->r_ob.playing)
							check_correct_scheduling((t_notation_obj *)x, true);
						invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
						return 1;
					} else {
						x->r_ob.show_loop_region = x->r_ob.show_loop_region ? false : true;
						send_loop_region_on_off((t_notation_obj *)x, 6);
						if (x->r_ob.use_loop_region && x->r_ob.show_loop_region)
							send_loop_region((t_notation_obj *)x, 6);
						if (x->r_ob.playing)
							check_correct_scheduling((t_notation_obj *)x, true);
						invalidate_notation_static_layer_and_repaint((t_notation_obj *)x);
						return 1;
					}
				} else {
					if (modifiers & eShiftKey) {
						// getting first selected chord
						if (!is_editable((t_notation_obj *)x, k_LYRICS, k_ELEMENT_ACTIONS_NONE))
							return 0;
						t_chord *ch = get_first_selected_chord((t_notation_obj *) x);
                        if (!ch) {
                            t_lyrics *ly = get_first_selected_lyrics((t_notation_obj *) x);
                            if (ly)
                                ch = ly->owner;
                        }
						if (ch && x->r_ob.show_lyrics && x->r_ob.link_lyrics_to_slot > 0)
							start_editing_lyrics((t_notation_obj *) x, patcherview, ch);
						invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
						return 1;
					} else if (x->r_ob.allow_lock){
						lock_unlock_selection((t_notation_obj *) x, true);
						handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_LOCK_UNLOCK_SELECTION); 
						invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
						return 1;
					}
				}
			}
			return 0;
			break;
            
        case 'd': // Cmd+Shift+D
            if (modifiers & eShiftKey && modifiers & eCommandKey) {
                // getting first selected chord
                if (!is_editable((t_notation_obj *)x, k_DYNAMICS, k_ELEMENT_ACTIONS_NONE))
                    return 0;
                t_chord *ch = get_first_selected_chord((t_notation_obj *) x);
                if (!ch) {
                    t_dynamics *dy = get_first_selected_dynamics((t_notation_obj *) x);
                    if (dy)
                        ch = dy->owner;
                }
                if (ch && x->r_ob.show_dynamics && x->r_ob.link_dynamics_to_slot > 0)
                    start_editing_dynamics((t_notation_obj *) x, patcherview, ch);
                invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
                return 1;
            }
            return 0;
            break;

            
		case 'u': // Cmd+U
			if (modifiers & eCommandKey && x->r_ob.allow_mute) {
				mute_unmute_selection((t_notation_obj *) x, true);
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_MUTE_UNMUTE_SELECTION); 
				return 1;
			}
			return 0;
			break;

		case 'j': // Cmd+J
			if (modifiers & eCommandKey && x->r_ob.allow_solo) {
				solo_unsolo_selection((t_notation_obj *) x, true);
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_SOLO_UNSOLO_SELECTION); 
				return 1;
			}
			return 0;
			break;
			
		case 'y': // Cmd+Y
			if (modifiers & eCommandKey && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET)) {
				align_selection_onsets(x);
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_ALIGN_SELECTION); 
				return 1;
			}
			return 0;
			break;
            
		case 'r': // Cmd+R
            if (modifiers & eCommandKey) {
                if (!is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_PITCH)) return 0;
                if (modifiers & eShiftKey){
                    if (is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_PITCH)) {
                        snap_pitch_to_grid_for_selection((t_notation_obj *) x);
                        handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_SNAP_PITCH_TO_GRID_FOR_SELECTION);
                    }
                    return 1;
                } else {
                    enharmonically_respell_selection((t_notation_obj *) x);
                    update_all_accidentals_if_needed(x);
                    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_RESET_ENHARMONICITY_FOR_SELECTION);
                    return 1;
                }
            }
			return 0;
			break;

		case 'g': // Cmd+G
			if (!(modifiers & eShiftKey)){
				if (!is_editable((t_notation_obj *)x, k_GROUP, k_ELEMENT_ACTIONS_NONE)) return 0;
				t_group *gr = NULL;
				if (x->r_ob.firstselecteditem){
					create_header_undo_tick((t_notation_obj *)x, k_HEADER_GROUPS);
					if (is_all_selection_in_one_group((t_notation_obj *) x, &gr)){
                        lock_general_mutex((t_notation_obj *)x);
						delete_group((t_notation_obj *) x, gr);
                        unlock_general_mutex((t_notation_obj *)x);
						handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_DELETE_GROUP);
					} else {
                        lock_general_mutex((t_notation_obj *)x);
						build_and_append_group_from_selection((t_notation_obj *) x);
                        unlock_general_mutex((t_notation_obj *)x);
						handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_CREATE_GROUP);
					}
					return 1;
				}
			}
			return 0;
			break;

		case 'a': // Cmd+A
			if (modifiers & eCommandKey && is_editable((t_notation_obj *)x, k_SELECTION, k_MULTIPLE_SELECTION)) {
				select_all(x);
				return 1;
			}
			return 0;
			break;

		case 'c': // Cmd+C
		case 'x': // Cmd+X
			if (modifiers & eCommandKey && x->r_ob.allow_copy_paste) {
				// copy or cut!
				if (x->r_ob.active_slot_num >= 0 && x->r_ob.active_slot_notationitem) {
					if (x->r_ob.selected_slot_items->l_size > 0) 
						notation_obj_copy_slot_selection((t_notation_obj *)x, &clipboard, x->r_ob.active_slot_notationitem, x->r_ob.active_slot_num, keycode == 'x');
					else
						notation_obj_copy_slot((t_notation_obj *)x, &clipboard, x->r_ob.active_slot_notationitem, modifiers & eShiftKey ? -1 : x->r_ob.active_slot_num, keycode == 'x');
				} else {
					roll_copy_selection(x, keycode == 'x');
				}
				return 1;
			}
			return 0;
			break;

		case 'v': case 'V': // letter V or v
			if (!(modifiers & eCommandKey))
				evaluate_selection(x, modifiers, true);
			else if (modifiers & eCommandKey && x->r_ob.allow_copy_paste && (clipboard.object == k_NOTATION_OBJECT_ROLL || clipboard.object == k_NOTATION_OBJECT_ANY) && clipboard.gathered_syntax && clipboard.gathered_syntax->l_size > 0) {
				// paste!
				if (clipboard.type == k_SLOT) {
					notation_obj_paste_slot((t_notation_obj *) x, &clipboard, (x->r_ob.active_slot_num < 0 || clipboard.gathered_syntax->l_size > 1) ? -1 : x->r_ob.active_slot_num);
				} else if (clipboard.type == k_SLOT_SELECTION) {
					notation_obj_paste_slot_selection_to_open_slot_window((t_notation_obj *) x, &clipboard, !(modifiers & eControlKey));
				} else if (clipboard.type == k_SELECTION_CONTENT) {
                    if (clipboard.object == k_NOTATION_OBJECT_ROLL && clipboard.gathered_syntax && clipboard.gathered_syntax->l_head) {
                        double onset = 0;
                        long voice = 0;
                        if (modifiers & eAltKey) {
                            onset = get_selection_leftmost_onset(x);
                            voice = get_selection_topmost_voice(x);
                            roll_delete_selection(x, false);
                        } else {
                            onset = xposition_to_onset((t_notation_obj *) x, x->r_ob.j_mouse_x, 0);
                            voice = yposition_to_voicenumber((t_notation_obj *)x, x->r_ob.j_mouse_y, -1, k_VOICEENSEMBLE_INTERFACE_ACTIVE);
                        }
                        if (voice >= 0)
                            roll_paste_clipboard(x, modifiers & eShiftKey, onset, false, voice, true, true);
                    }
				}
			}
			return 1;
			break;

		case 102: // Cmd+F
			if (modifiers & eCommandKey && is_editable((t_notation_obj *)x, k_NOTE_OR_CHORD, k_MODIFICATION_ONSET)) {
				equally_respace_selection_onsets(x);
				handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_EQUALLY_RESPACE_SELECTION);
				return 1;
			}
			return 0;
			break;

		case 112: // Cmd+P // show-hide playcursor
			if (modifiers & eCommandKey && is_editable((t_notation_obj *)x, k_PLAYCURSOR, k_ELEMENT_ACTIONS_NONE)) {
				x->r_ob.show_playhead = (x->r_ob.show_playhead) ? false : true;
                notationobj_redraw((t_notation_obj *) x);
				return 1;
			}
			return 0;
			break;
	} 
	
	if (textcharacter == 32 && x->r_ob.allow_play_from_interface){ // spacebar
		if (x->r_ob.playing) {
			if (modifiers == eShiftKey) { // acts like a "pause" command: playhead stays visible where it is
				x->r_ob.show_playhead = true;
				x->r_ob.play_head_start_ms = x->r_ob.play_head_ms;
                roll_stop(x, _llllobj_sym_pause, 0, NULL);
            } else
                roll_stop(x, NULL, 0, NULL);
		} else {
			if (modifiers == eShiftKey)
				roll_playselection(x, NULL, 0, NULL);
			else
				roll_play(x, NULL, 0, NULL);
		}
		return 1;
	}
	return 0;
}

	
long get_selection_topmost_voice(t_roll *x){
    double voice = 0;
    char def_voice = false;
    if (x->r_ob.firstselecteditem){
        t_notation_item *temp;
        for (temp = x->r_ob.firstselecteditem; temp; temp = temp->next_selected){
            long this_voice = -100;
            if (temp->type == k_NOTE || temp->type == k_CHORD || temp->type == k_MARKER)
                this_voice = notation_item_get_voicenumber((t_notation_obj *)x, temp);
            
            if (this_voice >= 0 && (!def_voice || this_voice < voice)) {
                def_voice = true;
                voice = this_voice;
            }
        }
    }
    return voice;
}


double get_selection_leftmost_onset(t_roll *x){
	double onset = 0;
	char def_onset = false;
	if (x->r_ob.firstselecteditem){
		t_notation_item *temp;
		for (temp = x->r_ob.firstselecteditem; temp; temp = temp->next_selected){
			double this_onset = -100;
			if (temp->type == k_NOTE || temp->type == k_CHORD || temp->type == k_MARKER)
                this_onset = notation_item_get_onset_ms((t_notation_obj *)x, temp);
			
			if (this_onset >= 0 && (!def_onset || this_onset < onset)) {
				def_onset = true;
				onset = this_onset;
			}
		}
	}
	return onset;
}

double get_selection_rightmost_onset(t_roll *x){
    double onset = 0;
    char def_onset = false;
    if (x->r_ob.firstselecteditem){
        t_notation_item *temp;
        for (temp = x->r_ob.firstselecteditem; temp; temp = temp->next_selected){
            double this_onset = -100;
            if (temp->type == k_NOTE || temp->type == k_CHORD || temp->type == k_MARKER)
                this_onset = notation_item_get_onset_ms((t_notation_obj *)x, temp) + notation_item_get_duration_ms((t_notation_obj *)x, temp);
            
            if (this_onset >= 0 && (!def_onset || this_onset > onset)) {
                def_onset = true;
                onset = this_onset;
            }
        }
    }
    return onset;
}

t_llll *get_selection_gathered_syntax(t_roll *x){
	t_llll *out = llll_get();
	t_rollvoice *voice;
	for (voice = x->firstvoice; voice && voice->v_ob.number < x->r_ob.num_voices; voice = voice->next){
		t_chord *chord;
		t_llll *voice_llll = llll_get();
		for (chord = voice->firstchord; chord; chord = chord->next){
			char there_is_something_selected = false;
			if (notation_item_is_globally_selected((t_notation_obj *) x, (t_notation_item *)chord))
				there_is_something_selected = true;
			else {
				t_note *nt;
				for (nt = chord->firstnote; nt; nt = nt->next){
					if (notation_item_is_globally_selected((t_notation_obj *) x, (t_notation_item *)nt)) {
						there_is_something_selected = true;
						break;
					}
				}
			}
			if (there_is_something_selected)
				llll_appendllll(voice_llll, get_rollchord_values_as_llll((t_notation_obj *) x, chord, k_CONSIDER_FOR_PLAYING_ONLY_IF_SELECTED), 0, WHITENULL_llll);
		}
		llll_appendllll(out, voice_llll, 0, WHITENULL_llll);
	}
	return out;
}
	

void evaluate_selection(t_roll *x, long modifiers, char alsosortselectionbyonset, t_llll *forced_routers)
{
    if (alsosortselectionbyonset)
        sort_selection((t_notation_obj *)x, false);
    
	// detect the selection type
	if ((modifiers & eShiftKey) && (modifiers & eAltKey)) { // send all values
		send_all_values_as_llll(x, k_HEADER_ALL); // dump all
	} else if (modifiers & eShiftKey) { // send chord values
		if (x->r_ob.num_selecteditems == 1 && x->r_ob.firstselecteditem->type == k_NOTE)
			send_chord_as_llll((t_notation_obj *) x, ((t_note *)x->r_ob.firstselecteditem)->parent, 6, k_CONSIDER_FOR_EVALUATION, -1, forced_routers);
		else if (x->r_ob.num_selecteditems == 1 && x->r_ob.firstselecteditem->type == k_CHORD)
			send_chord_as_llll((t_notation_obj *) x, (t_chord *)x->r_ob.firstselecteditem, 6, k_CONSIDER_FOR_EVALUATION, -1, forced_routers);
	} else { // send selection values
		standard_dump_selection((t_notation_obj *) x, 6, -1, (delete_item_fn)roll_sel_delete_item, forced_routers);
	}
}

void selection_send_command(t_roll *x, long modifiers, long command_number, char alsosortselectionbyonset)
{
    if (alsosortselectionbyonset)
        sort_selection((t_notation_obj *)x, false);

	if (command_number == -1 && modifiers & eShiftKey) {
        // chord-wise dump but not for commands: we might wanna define a command for Shift+Something!
		if ((x->r_ob.num_selecteditems == 1) && (x->r_ob.firstselecteditem->type == k_NOTE))
			send_chord_as_llll((t_notation_obj *) x, ((t_note *)x->r_ob.firstselecteditem)->parent, 6, k_CONSIDER_FOR_EVALUATION, command_number);
		else if ((x->r_ob.num_selecteditems == 1) && (x->r_ob.firstselecteditem->type == k_CHORD))
			send_chord_as_llll((t_notation_obj *) x, (t_chord *)x->r_ob.firstselecteditem, 6, k_CONSIDER_FOR_EVALUATION, command_number);
	} else { // send selection values
		standard_dump_selection((t_notation_obj *)x, 6, command_number, (delete_item_fn)roll_sel_delete_item);
	}
}

char move_selection_breakpoint(t_roll *x, double delta_x_pos, double delta_y_pos, char tail_only){
	t_notation_item *curr_it = x->r_ob.firstselecteditem;
	char changed = 0;
	lock_general_mutex((t_notation_obj *)x);	
	while (curr_it) { // cycle on the selected items
		if (curr_it->type == k_PITCH_BREAKPOINT && ((t_bpt *)curr_it)->next && !tail_only) { // it is a breakpoint : let's move it
			t_note *note = ((t_bpt *)curr_it)->owner;
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)note)) {
				double note_length = onset_to_xposition((t_notation_obj *) x, note->parent->onset + note->duration, NULL) - onset_to_xposition((t_notation_obj *) x, note->parent->onset, NULL);
				double delta_rel_x_pos = delta_x_pos / note_length;
				
				if (!(note->parent->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
					create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)note->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);

				move_breakpoint((t_notation_obj *) x, (t_bpt *)curr_it, delta_rel_x_pos, delta_y_pos);
				changed = 1;
			}
		} else if (curr_it->type == k_PITCH_BREAKPOINT && !((t_bpt *)curr_it)->next) {
			t_note *note = ((t_bpt *)curr_it)->owner;
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)note)) {
//				double delta_ms =  xposition_to_onset((t_notation_obj *) x, delta_x_pos, 0) - xposition_to_onset((t_notation_obj *) x, 0, 0);
				double delta_ms =  deltaxpixels_to_deltaonset((t_notation_obj *)x, delta_x_pos);

				if (!(note->parent->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
					create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)note->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
				
				move_breakpoint((t_notation_obj *) x, note->lastbreakpoint, 0., delta_y_pos);
				change_note_duration(x, note, delta_ms);
				changed = 1;
			}
		}
		curr_it = curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);	
	return changed;
}

char change_selection_onset(t_roll *x, double *delta_ms)
{
	t_notation_item *curr_it;
	char changed = false; 
	char gotta_check_chords_order[CONST_MAX_VOICES];
    char gotta_check_markers_order = false;
	t_rollvoice *voice;
	long i; 
	for (i=0; i<CONST_MAX_VOICES; i++) 
		gotta_check_chords_order[i]=false;
	
	lock_general_mutex((t_notation_obj *)x);

	// First of all: we detect if we should actually change anything: 0ms is indeed a barrier, and if we are dragging leftwards stuff which
	// has already hit the barrier, nothing should happen
	for (curr_it = x->r_ob.firstselecteditem; curr_it; curr_it = curr_it->next_selected) {
		double this_onset = notation_item_get_onset_ms((t_notation_obj *)x, curr_it);
		if (this_onset + *delta_ms < 0) 
			*delta_ms = -this_onset; // < we modify the delta onset, should an element be nearer the barrier than the required delta onset
		else if (curr_it->type == k_PITCH_BREAKPOINT && this_onset + *delta_ms < ((t_bpt *)curr_it)->owner->parent->onset)
			*delta_ms = ((t_bpt *)curr_it)->owner->parent->onset - this_onset; // < idem, for pitch breakpoints, which should not be dragged BEFORE the corresponding notehead
	}
	
	if (*delta_ms != 0) {
		
		// clearing modified flag
		for (curr_it = x->r_ob.firstselecteditem; curr_it; curr_it = curr_it->next_selected) {
			if (curr_it->type == k_NOTE)
				((t_note *) curr_it)->parent->r_it.flags = (e_bach_internal_notation_flags) (((t_note *) curr_it)->parent->r_it.flags & ~k_FLAG_MODIFIED);
			else if (curr_it->type == k_CHORD)
				((t_chord *) curr_it)->r_it.flags = (e_bach_internal_notation_flags) (((t_chord *) curr_it)->r_it.flags & ~k_FLAG_MODIFIED);
		}
		
		// the k_FLAG_MODIFIED flag is a local flag keeping track of selection elements already modified.
		// on the other hand, when a chord is modified, its flag k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER is set, so that 
		// if we are dragging it, an undo tick is created only the FIRST time the chord is dragged and not always during mousedrag
		
		// cycle on selected items for modification
		for (curr_it = x->r_ob.firstselecteditem; curr_it; curr_it = curr_it->next_selected) {
			if (curr_it->type == k_NOTE) { // it is a note
				t_chord *ch = ((t_note *)curr_it)->parent;
				if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)ch) && !(ch->r_it.flags & k_FLAG_MODIFIED)) {
					
					if (!(ch->r_it.flags & k_FLAG_MODIF_CHECK_ORDER_UNDO))
						create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)ch, k_CHORD, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
					
					ch->onset += *delta_ms;
					if (ch->onset < 0) 
						ch->onset = 0;
					gotta_check_chords_order[CLAMP(ch->voiceparent->v_ob.number, 0, CONST_MAX_VOICES - 1)] = true;
					changed = 1;
					ch->r_it.flags = (e_bach_internal_notation_flags) (ch->r_it.flags | k_FLAG_MODIFIED);
                    update_all_accidentals_for_chord_if_needed(x, ch);
				}
			} else if (curr_it->type == k_CHORD) {
				t_chord *ch = ((t_chord *)curr_it);
				if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)ch) && !(ch->r_it.flags & k_FLAG_MODIFIED)) {
					
					if (!(ch->r_it.flags & k_FLAG_MODIF_CHECK_ORDER_UNDO))
						create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)ch, k_CHORD, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
					
					ch->onset += *delta_ms; 
					if (ch->onset < 0) 
						ch->onset = 0;
					gotta_check_chords_order[CLAMP(ch->voiceparent->v_ob.number, 0, CONST_MAX_VOICES - 1)] = true;
					ch->r_it.flags = (e_bach_internal_notation_flags) (e_bach_internal_notation_flags) (ch->r_it.flags | k_FLAG_MODIFIED);
					changed = 1;
                    update_all_accidentals_for_chord_if_needed(x, ch);
				}
			} else if (curr_it->type == k_PITCH_BREAKPOINT && !((t_bpt *)curr_it)->next) { // it is a note tail
				t_note *nt = ((t_bpt *)curr_it)->owner;
				if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt) && !(nt->r_it.flags & k_FLAG_MODIFIED)) {
					
					if (!(nt->parent->r_it.flags & k_FLAG_MODIFIED))
						create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)nt->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
					
					nt->duration += *delta_ms;
					if (nt->duration < 0) 
						nt->duration = 0;
					
					changed = 1;
					nt->parent->r_it.flags = (e_bach_internal_notation_flags) (nt->parent->r_it.flags | k_FLAG_MODIFIED);
				}		
			} else if (curr_it->type == k_MARKER) {
				t_marker *marker = ((t_marker *)curr_it);
				
				if (!(x->r_ob.header_undo_flags & k_HEADER_MARKERS)) {
					create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
					x->r_ob.header_undo_flags |= k_HEADER_MARKERS;
				}
				
				marker->position_ms += *delta_ms;
                gotta_check_markers_order = true;
				changed = 1;
			}
		}
		
		// clearing modified flag
		for (curr_it = x->r_ob.firstselecteditem; curr_it; curr_it = curr_it->next_selected) {
			if (curr_it->type == k_NOTE)
				((t_note *) curr_it)->parent->r_it.flags = (e_bach_internal_notation_flags) (((t_note *) curr_it)->parent->r_it.flags & ~k_FLAG_MODIFIED);
			else if (curr_it->type == k_CHORD)
				((t_chord *) curr_it)->r_it.flags = (e_bach_internal_notation_flags) (((t_chord *) curr_it)->r_it.flags & ~k_FLAG_MODIFIED);
			else if (curr_it->type == k_PITCH_BREAKPOINT)
				(((t_bpt *)curr_it)->owner)->parent->r_it.flags = (e_bach_internal_notation_flags) ((((t_bpt *)curr_it)->owner)->parent->r_it.flags & ~k_FLAG_MODIFIED);
		}
		
		for (voice = x->firstvoice; voice && (voice->v_ob.number < x->r_ob.num_voices); voice = voice->next)
			if (gotta_check_chords_order[voice->v_ob.number])
				check_chords_order_for_voice(x, voice);
        
        if (gotta_check_markers_order)
            check_markers_order((t_notation_obj *)x);
	}
	
	if (changed)
		recompute_total_length((t_notation_obj *)x);
	
	unlock_general_mutex((t_notation_obj *)x);	

	if (changed)
		check_correct_scheduling((t_notation_obj *)x, true);
	
	return changed;
}



void change_note_duration(t_roll *x, t_note *note, double delta_ms){  
	if (notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)note)) 
		return;

	note->duration += delta_ms;
	if (note->duration < 0) 
		note->duration = 0;
	recompute_total_length((t_notation_obj *)x);
}

char change_selection_duration(t_roll *x, double delta_ms){
	t_notation_item *curr_it = x->r_ob.firstselecteditem;
	char changed = 0;
	lock_general_mutex((t_notation_obj *)x);
	while (curr_it) { // cycle on the selected items
		if (curr_it->type == k_NOTE || (curr_it->type == k_PITCH_BREAKPOINT && !((t_bpt *)curr_it)->next)) { // it is a note (I might have selected the duration tail, but however...)
			t_note *note = curr_it->type == k_NOTE ? (t_note *)curr_it : ((t_bpt *)curr_it)->owner;
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)note)) {
				
				if (!(note->parent->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
					create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)note->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);
				
				note->duration += delta_ms; 
				if (note->duration < 0) 
					note->duration = 0;
				changed = 1;
			}
		} else if (curr_it->type == k_CHORD) {
			t_note *curr_nt = ((t_chord *)curr_it)->firstnote;
			while (curr_nt) { // cycle on the notes
				if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)curr_nt)) {

					if (!(curr_nt->parent->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
						create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)curr_nt->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);

					curr_nt->duration += delta_ms; 
					if (curr_nt->duration < 0) 
						curr_nt->duration = 0;
				}
				curr_nt = curr_nt->next;
			}
			changed = 1;
		}
		curr_it = curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);
	if (changed)
		recompute_total_length((t_notation_obj *)x);
	return changed;
}






/*
char change_selection_tail(t_roll *x, double delta_ms){
	t_notationel *curr_it = x->r_ob.firstselecteditem;
	char changed = 0;
	while (curr_it) { // cycle on the selected items
		if (curr_it->type == k_DURATION_TAIL) { // it is a note
			t_note *note = ((t_note *)curr_it->item);
			if (!note->locked && !note->parent->locked) {
				((t_note *)curr_it->item)->duration += delta_ms; 
				if (((t_note *)curr_it->item)->duration < 0) ((t_note *)curr_it->item)->duration = 0;
				recompute_total_length((t_notation_obj *)x, k_NOTATION_OBJECT_ROLL, x->firstvoice, x->r_ob.num_voices);
				changed = 1;
			}
		}
		curr_it = curr_it->next;
	}
	return changed;
}
*/

char align_selection_onsets(t_roll *x){
//align all the onsets to the first onset

	// find leftmost onset
	t_rollvoice *voice;
	t_notation_item *curr_it = x->r_ob.firstselecteditem;
	char changed = 0;
	long leftmost_onset = -32000; // local default for non-defined
	
	lock_general_mutex((t_notation_obj *)x);
	while (curr_it) { // cycle on the selected items
		if (curr_it->type == k_NOTE) { // it is a note
			if ((leftmost_onset == -32000) || (((t_note *)curr_it)->parent->onset < leftmost_onset)) 
				leftmost_onset = ((t_note *)curr_it)->parent->onset;
		} else if (curr_it->type == k_CHORD) { // it is a chord
			if ((leftmost_onset == -32000) || (((t_chord *)curr_it)->onset < leftmost_onset)) 
				leftmost_onset = ((t_chord *)curr_it)->onset;
		} else if (curr_it->type == k_PITCH_BREAKPOINT && !((t_bpt *)curr_it)->next) { // it is a note tail
			double thisonset = get_breakpoint_absolute_onset((t_bpt *) curr_it);
			if (leftmost_onset == -32000 || thisonset < leftmost_onset) 
				leftmost_onset = thisonset;
		}
		curr_it = curr_it->next_selected;
	}
	
	//changing onsets
	curr_it = x->r_ob.firstselecteditem;
	while (curr_it) { // cycle on the selected items
		if (curr_it->type == k_NOTE) { // it is a note
			t_note *note = ((t_note *)curr_it);
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)note)) {
				create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)note->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
				((t_note *)curr_it)->parent->onset = leftmost_onset; 
				changed = 1;
			}
		} else if (curr_it->type == k_CHORD) { // it is a chord
			t_chord *chord = ((t_chord *)curr_it);
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)chord)) {
				create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)chord, k_CHORD, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
				((t_chord *)curr_it)->onset = leftmost_onset; 
				changed = 1;
			}
		} else if (curr_it->type == k_PITCH_BREAKPOINT && !((t_bpt *)curr_it)->next) { // it is a note tail
			t_bpt *bpt = (t_bpt *)curr_it;
			if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)bpt->owner)) {
				create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)bpt->owner->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
				bpt->owner->duration = MAX(0, leftmost_onset - bpt->owner->parent->onset);
				changed = 1;
			}
		}
		curr_it = curr_it->next_selected;
	}
	
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		check_chords_order_for_voice(x, voice);
		voice = voice->next;
	}
	unlock_general_mutex((t_notation_obj *)x);

	recompute_total_length((t_notation_obj *)x);
	return changed;
}

void roll_distribute(t_roll *x){
	equally_respace_selection_onsets(x);
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER_AND_BANG, k_UNDO_OP_EQUALLY_RESPACE_SELECTION);
}

char equally_respace_selection_onsets(t_roll *x){
//equally respace all the onsets of the selection  
	char changed = false;
	t_rollvoice *voice;
	t_notation_item *curr_it;

	// NEW CODE
	t_llll **onsets_and_chords = (t_llll **) bach_newptr(2 * sizeof(t_llll *));

	lock_general_mutex((t_notation_obj *)x);
	
	onsets_and_chords[0] = llll_get();
	onsets_and_chords[1] = llll_get();
	for (curr_it = x->r_ob.firstselecteditem; curr_it; curr_it = curr_it->next_selected) { // cycle on the selected items
		if (curr_it->type == k_NOTE) { // it is a note
			llll_appendobj(onsets_and_chords[1], ((t_note *)curr_it)->parent, 0, WHITENULL_llll);
			llll_appenddouble(onsets_and_chords[0], ((t_note *)curr_it)->parent->onset, 0, WHITENULL_llll);
			changed = true;
		} else if (curr_it->type == k_CHORD) { // it is a chord
			llll_appendobj(onsets_and_chords[1], curr_it, 0, WHITENULL_llll);
			llll_appenddouble(onsets_and_chords[0], ((t_chord *)curr_it)->onset, 0, WHITENULL_llll);
			changed = true;
        } else if (curr_it->type == k_MARKER) { // it is a marker
            llll_appendobj(onsets_and_chords[1], curr_it, 0, WHITENULL_llll);
            llll_appenddouble(onsets_and_chords[0], ((t_marker *)curr_it)->position_ms, 0, WHITENULL_llll);
            changed = true;
		}
	}
	
/*	llll_post(onsets_and_chords[0], 0, 1, 2, NULL, NULL);
	llll_post(onsets_and_chords[1], 0, 1, 2, NULL, NULL);*/

	llll_multisort(onsets_and_chords, onsets_and_chords, 2, (sort_fn)llll_leq_elem);

/*	post("sorted");
	llll_post(onsets_and_chords[0], 0, 1, 2, NULL, NULL);
	llll_post(onsets_and_chords[1], 0, 1, 2, NULL, NULL);*/
	
	if (onsets_and_chords[0] && onsets_and_chords[0]->l_size > 1){
		t_llllelem *elem;
		double step = (hatom_getdouble(&onsets_and_chords[0]->l_tail->l_hatom) - hatom_getdouble(&onsets_and_chords[0]->l_head->l_hatom)) / (onsets_and_chords[0]->l_size - 1);
		double this_onset;
		changed = true;
		for (elem = onsets_and_chords[1]->l_head, this_onset = hatom_getdouble(&onsets_and_chords[0]->l_head->l_hatom); 
			 elem; 
			 elem = elem->l_next, this_onset += step){
            t_notation_item *item = (t_notation_item *)hatom_getobj(&elem->l_hatom);
            if (item->type == k_CHORD) {
                t_chord *ch = (t_chord *)item;
                if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)ch)) {
                    create_simple_selected_notation_item_undo_tick((t_notation_obj *)x, (t_notation_item *)ch, k_CHORD, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
                    ch->onset = this_onset;
                    ch->r_it.flags = (e_bach_internal_notation_flags) (ch->r_it.flags & ~k_FLAG_COUNT);
                }
            } else if (item->type == k_MARKER) {
                t_marker *mk = (t_marker *)item;
                if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)mk)) {
                    create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
                    mk->position_ms = this_onset;
                    mk->r_it.flags = (e_bach_internal_notation_flags) (mk->r_it.flags & ~k_FLAG_COUNT);
                }
            }
		}
	}
	
	if (onsets_and_chords[0]) 
		llll_free(onsets_and_chords[0]);
	if (onsets_and_chords[1]) 
		llll_free(onsets_and_chords[1]);

	bach_freeptr(onsets_and_chords);
	
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		check_chords_order_for_voice(x, voice);
		voice = voice->next;
	}
	
	unlock_general_mutex((t_notation_obj *)x);

	recompute_total_length((t_notation_obj *)x);
	return changed;
}


void select_all(t_roll *x){
	t_rollvoice *voice = x->firstvoice;
	t_chord *chord;
	lock_general_mutex((t_notation_obj *)x);	
	clear_selection((t_notation_obj *) x);
	clear_preselection((t_notation_obj *) x);
	voice = x->firstvoice;
	while (voice && (voice->v_ob.number < x->r_ob.num_voices)) {
		chord = voice->firstchord;
		while (chord) {
			notation_item_add_to_selection((t_notation_obj *) x, (t_notation_item *)chord);
			chord = chord->next;
		}
		voice = voice->next;
	}
	lock_markers_mutex((t_notation_obj *)x);
	select_all_markers((t_notation_obj *)x, k_SELECTION_MODE_FORCE_SELECT);
	unlock_markers_mutex((t_notation_obj *)x);
	unlock_general_mutex((t_notation_obj *)x);
	handle_change_selection((t_notation_obj *)x);
//	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
}	




// does NOT work with markers
// and beware this is thought to be used for mouse interaction, so v1 and v2 are unintuitively wrong...
// v1 is the voice corresponding to mc1 and v2 is the voice orresponding to mc2. I.e. v1 >= 2
// whereas mc1 <= mc2. This is unintuitive. Use the function above for most programmatical purposes
void preselect_elements_in_region_for_mouse_selection(t_roll *x, double ms1, double ms2, double mc1, double mc2, long v1, long v2){
	// preselect (and then, at the mouseup: will select) all the elements in the region

	t_rollvoice *voice;
	t_chord *curr_chord; 
	long voicenum;
	
    for (voice = x->firstvoice; voice && (voice->v_ob.number < x->r_ob.num_voices); voice = voice->next) {

        if (voice->v_ob.hidden)
            continue;
        
	//	long count_notes_in_region;
		voicenum = voice->v_ob.number;
        
        // correcting voicenum for voiceensembles
        char same_v1_v2 = (v1 == v2);
        t_voice *v1v = nth_voice((t_notation_obj *)x, v1);
        t_voice *v2v = nth_voice((t_notation_obj *)x, v2);
        if (do_voices_belong_to_same_voiceensemble((t_notation_obj *)x, (t_voice *)voice, v1v))
            voicenum = v1;
        else if (do_voices_belong_to_same_voiceensemble((t_notation_obj *)x, (t_voice *)voice, v2v))
            voicenum = v2;
        if (do_voices_belong_to_same_voiceensemble((t_notation_obj *)x, v1v, v2v))
            same_v1_v2 = true;

		curr_chord = voice->firstchord;
		//	post("----", curr_chord);
		while (curr_chord) {
			if ((curr_chord->onset >= ms1)&&(curr_chord->onset <= ms2)){
				t_note *curr_nt = curr_chord->firstnote;
				while (curr_nt) { // cycle on the notes
					if ( (voicenum > v2 && voicenum < v1) ||
						(same_v1_v2 && (v1 == voicenum) && ((curr_nt->midicents >= mc1)&&(curr_nt->midicents <= mc2))) ||
						(!same_v1_v2 && (voicenum == v1) && (curr_nt->midicents >= mc1)) ||
						(!same_v1_v2 && (voicenum == v2) && (curr_nt->midicents <= mc2)) ) {
						//						post("adding %lx to preselection", curr_nt);
						notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)curr_nt);
					}
					curr_nt = curr_nt->next;
				}
				//				}
			} else if ((x->r_ob.show_durations) && (curr_chord->onset < ms1)) { // maybe the duration tail is selected...
				t_note *curr_nt = curr_chord->firstnote;
				while (curr_nt) { // cycle on the notes
					if ( ((curr_chord->onset+curr_nt->duration >= ms1)&&(curr_chord->onset+curr_nt->duration <= ms2)) &&
						 (  ((voicenum > v2) && (voicenum < v1)) ||
							(same_v1_v2 && (v1 == voicenum) && ((curr_nt->midicents + curr_nt->lastbreakpoint->delta_mc >= mc1)&&(curr_nt->midicents + curr_nt->lastbreakpoint->delta_mc <= mc2))) ||
							(!same_v1_v2 && (voicenum == v1) && (curr_nt->midicents + curr_nt->lastbreakpoint->delta_mc >= mc1)) ||
							(!same_v1_v2 && (voicenum == v2) && (curr_nt->midicents + curr_nt->lastbreakpoint->delta_mc <= mc2)) )) {
						notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)curr_nt->lastbreakpoint);
					}
					curr_nt = curr_nt->next;
				}
			}
			if ((x->r_ob.show_durations) && (!(curr_chord->onset >= ms2)) && (!notation_item_is_preselected((t_notation_obj *)x, (t_notation_item *)curr_chord))) { // looking for breakpoint-selection
				t_note *curr_nt = curr_chord->firstnote;
				while (curr_nt) { // cycle on the notes
					// we only select breakpoints IF the whole note has NOT been selected in the region, otherwise the whole note is selected, that's all
					if (!notation_item_is_preselected((t_notation_obj *)x, (t_notation_item *)curr_nt)) {
						t_bpt *curr_bpt = curr_nt->firstbreakpoint->next;
						while (curr_bpt) { // cycle on the breakpoints
							if (curr_bpt != curr_nt->lastbreakpoint){
								if ( ((curr_chord->onset + curr_bpt->rel_x_pos * curr_nt->duration >= ms1)&&(curr_chord->onset + curr_bpt->rel_x_pos * curr_nt->duration <= ms2)) &&
									(  ((voicenum > v2) && (voicenum < v1)) ||
									 (same_v1_v2 && (v1 == voicenum) && ((curr_nt->midicents + curr_bpt->delta_mc >= mc1)&&(curr_nt->midicents + curr_bpt->delta_mc <= mc2))) ||
									 (!same_v1_v2 && (voicenum == v1) && (curr_nt->midicents + curr_bpt->delta_mc >= mc1)) ||
									 (!same_v1_v2 && (voicenum == v2) && (curr_nt->midicents + curr_bpt->delta_mc <= mc2)) )) {
										notation_item_add_to_preselection((t_notation_obj *) x, (t_notation_item *)curr_bpt);						
									}
							}
							curr_bpt = curr_bpt->next;
						}
					}
					curr_nt = curr_nt->next;
				}
				
			} 
			curr_chord = curr_chord->next;
		} 
	}
}

char is_chord_in_selected_region(t_roll *x, t_chord *chord){
// determines whether a chord is within the selected region. Return 0 if chord is completely outside the region, 1 if the entire chord is within the region, 2 if some notes are inside, some outside
	long ms1 = x->r_ob.j_selected_region_ms1;
	long ms2 = x->r_ob.j_selected_region_ms2;
	long mc1 = x->r_ob.j_selected_region_mc1; // low
	long mc2 = x->r_ob.j_selected_region_mc2; // high
	long v1 = x->r_ob.j_selected_region_voice1;
	long v2 = x->r_ob.j_selected_region_voice2;
	
	long count_notes_in_region = 0;
	long chord_voice = chord->voiceparent->v_ob.number;
//	post("°°°°°°°°°°°°°°°°° chord: %lx. Num_notes: %d", chord, chord->num_notes);
	if ((chord->onset >= ms1)&&(chord->onset <= ms2)){
		if ((chord_voice >= v1)&&(chord_voice <= v2)){ // voice can be ok
			t_note *curr_nt = chord->firstnote;
			while (curr_nt) { // cycle on the notes
				if ( ((chord_voice > v1) && (chord_voice < v2)) ||
					 ((v1 == v2) && ((curr_nt->midicents >= mc1)&&(curr_nt->midicents <= mc2))) ||
					 ((chord_voice == v1) && (curr_nt->midicents <= mc2)) ||
					 ((chord_voice == v2) && (curr_nt->midicents >= mc1)) )
					count_notes_in_region++;
				curr_nt = curr_nt->next;
			}
			if (count_notes_in_region == chord->num_notes){
				return 1; // totally inside
			} else {
				return 2; // partially inside
			}
		}
	} else {
		return 0; // totally outside
	}
	
	return 0; // BIZZARRO... da mettere
}

char is_note_in_selected_region(t_roll *x, t_chord *chord, t_note *note){
// determines whether a note of a given chord is within the selected region. Return 0 if outside, 1 if inside
	long ms1 = x->r_ob.j_selected_region_ms1;
	long ms2 = x->r_ob.j_selected_region_ms2;
	long mc1 = x->r_ob.j_selected_region_mc1;
	long mc2 = x->r_ob.j_selected_region_mc2;
	long v1 = x->r_ob.j_selected_region_voice1;
	long v2 = x->r_ob.j_selected_region_voice2;

	long chord_voice = chord->voiceparent->v_ob.number;
	if ( ((chord->onset >= ms1) &&(chord->onset <= ms2)) &&
		 ( ((chord_voice > v1) && (chord_voice < v2)) ||
	     	((v1 == v2) && ((note->midicents >= mc1)&&(note->midicents <= mc2))) ||
			((chord_voice == v1) && (note->midicents <= mc2)) ||
			((chord_voice == v2) && (note->midicents >= mc1)) ) )
		return 1;
	else
		return 0;
}

void roll_focusgained(t_roll *x, t_object *patcherview) {
	x->r_ob.j_has_focus = true;
    notationobj_redraw((t_notation_obj *) x);
}

void roll_focuslost(t_roll *x, t_object *patcherview) {
    set_mousedown((t_notation_obj *)x, NULL, k_NONE, false);

	if (x->r_ob.show_dilation_rectangle) {
		x->r_ob.show_dilation_rectangle = false;
		if (x->r_ob.j_mouse_cursor == BACH_CURSOR_DRAGGINGHAND) 
			bach_set_cursor((t_object *)x, &x->r_ob.j_mouse_cursor, patcherview, BACH_CURSOR_DEFAULT);
	}
		
	if (!x->r_ob.keep_selection_if_lost_focus){
		lock_general_mutex((t_notation_obj *)x);
		clear_selection((t_notation_obj *) x);
		unlock_general_mutex((t_notation_obj *)x);
	}
	x->r_ob.j_has_focus = false;
}


char roll_sel_dilate_ms(t_roll *x, double ms_factor, double fixed_ms_point){
	char changed = 0;
	t_notation_item *curr_it;
	
	if (ms_factor == 1.)
		return 0;
		
	lock_general_mutex((t_notation_obj *)x);
	curr_it = x->r_ob.firstselecteditem;
	while (curr_it) {
		if (curr_it->type == k_NOTE) {
			t_note *nt = (t_note *) curr_it;
			t_chord *chord = nt->parent;
            double old_onset = chord->onset;
            if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)){
                if (!(nt->parent->r_it.flags & k_FLAG_MODIF_CHECK_ORDER_UNDO))
                    create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)nt->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
                
                chord->onset = fixed_ms_point + (chord->onset - fixed_ms_point) * ms_factor;
                if (old_onset + nt->duration <= x->r_ob.dilation_rectangle.right_ms)
                    nt->duration = nt->duration * ms_factor;
                else if (old_onset <= x->r_ob.dilation_rectangle.right_ms)
                    nt->duration = (old_onset + nt->duration - x->r_ob.dilation_rectangle.right_ms) + (x->r_ob.dilation_rectangle.right_ms - old_onset) * ms_factor;
                changed = 1;
            }
		} if (curr_it->type == k_PITCH_BREAKPOINT && !((t_bpt *)curr_it)->next) {
            t_note *nt = ((t_bpt *)curr_it)->owner;
            if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)nt)){
                double old_onset = nt->parent->onset;
                
                if (!(nt->parent->r_it.flags & k_FLAG_MODIF_CHECK_ORDER_UNDO))
                    create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)nt->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
                
                if (old_onset + nt->duration >= x->r_ob.dilation_rectangle.left_ms)
                    nt->duration = x->r_ob.dilation_rectangle.left_ms + (old_onset + nt->duration - x->r_ob.dilation_rectangle.left_ms) * ms_factor - old_onset;
                changed = 1;
            }
		} else if (curr_it->type == k_CHORD) {
            t_chord *chord = (t_chord *) curr_it;
            if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)chord)){
                t_note *nt;
                double old_onset = chord->onset;
                
                if (!(chord->r_it.flags & k_FLAG_MODIF_CHECK_ORDER_UNDO))
                    create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)chord, k_CHORD, k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER);
                
                chord->onset = fixed_ms_point + (chord->onset - fixed_ms_point) * ms_factor;
                changed = 1;
                for (nt = chord->firstnote; nt; nt = nt->next){
                    if (old_onset + nt->duration <= x->r_ob.dilation_rectangle.right_ms)
                        nt->duration = nt->duration * ms_factor;
                    else if (old_onset <= x->r_ob.dilation_rectangle.right_ms)
                        nt->duration = (old_onset + nt->duration - x->r_ob.dilation_rectangle.right_ms) + (x->r_ob.dilation_rectangle.right_ms - old_onset) * ms_factor;
                }
            }
		} else if (curr_it->type == k_MARKER) {
			t_marker *marker = (t_marker *) curr_it;
			double ms = marker->position_ms;

            if (!notation_item_is_globally_locked((t_notation_obj *)x, (t_notation_item *)marker)){
                if (!(x->r_ob.header_undo_flags & k_HEADER_MARKERS)) {
                    create_header_undo_tick((t_notation_obj *)x, k_HEADER_MARKERS);
                    x->r_ob.header_undo_flags |= k_HEADER_MARKERS;
                }
                
                ms = fixed_ms_point + (ms - fixed_ms_point) * ms_factor;
                change_marker_ms((t_notation_obj *) x, marker, ms, 0, 0);
                changed = 1;
            }
		}
		curr_it = curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);

	// chord order check and length update are only done at the mouseup 
	return changed;
}


char roll_sel_dilate_mc(t_roll *x, double mc_factor, double fixed_mc_y_pixel){
	char changed = 0;
	t_notation_item *curr_it;
	
	if (mc_factor == 1.)
		return 0;
	
	lock_general_mutex((t_notation_obj *)x);
	curr_it = x->r_ob.firstselecteditem;
	while (curr_it) {
		if (curr_it->type == k_NOTE) {
			t_note *nt = (t_note *) curr_it;
			double fixed_mc_point = yposition_to_mc((t_notation_obj *)x, fixed_mc_y_pixel, (t_voice *)nt->parent->voiceparent, NULL);
			
			if (!(nt->parent->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
				create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)nt->parent, k_CHORD, k_UNDO_MODIFICATION_CHANGE);

			nt->midicents = fixed_mc_point + (nt->midicents - fixed_mc_point) * mc_factor;
			if (nt->midicents < 0)
				nt->midicents = 0;
			nt->parent->need_recompute_parameters = true;
			changed = 1;
		} else if (curr_it->type == k_CHORD) {
			t_chord *chord = (t_chord *) curr_it;
			double fixed_mc_point = yposition_to_mc((t_notation_obj *)x, fixed_mc_y_pixel, (t_voice *)chord->voiceparent, NULL);
			
			if (!(chord->r_it.flags & k_FLAG_MODIF_UNDO_WITH_OR_WO_CHECK_ORDER))
				create_simple_selected_notation_item_undo_tick((t_notation_obj *) x, (t_notation_item *)chord, k_CHORD, k_UNDO_MODIFICATION_CHANGE);

			t_note *nt;
			for (nt = chord->firstnote; nt; nt = nt->next) {
//				double old_mc = nt->midicents;
				nt->midicents = fixed_mc_point + (nt->midicents - fixed_mc_point) * mc_factor;
				if (nt->midicents < 0)
					nt->midicents = 0;
//				dev_post("%.2f = %.2f + (%.2f - %.2f) * %.2f", nt->midicents, fixed_mc_point, old_mc, fixed_mc_point, mc_factor);
			}
			chord->need_recompute_parameters = true;
			changed = 1;
		}
		curr_it = curr_it->next_selected;
	}
	unlock_general_mutex((t_notation_obj *)x);
	
	return changed;
}


void roll_old_redo(t_roll *x){
	if (x->r_ob.old_redo_llll[0]) {
		int i;
		set_roll_from_llll(x, x->r_ob.old_redo_llll[0], true); // resetting roll
		
		lock_general_mutex((t_notation_obj *)x);	
		for (i = CONST_MAX_UNDO_STEPS - 1; i > 0; i--) // reshifting all undo elements
			x->r_ob.old_undo_llll[i]=x->r_ob.old_undo_llll[i-1];
		x->r_ob.old_undo_llll[0] = x->r_ob.old_redo_llll[0];
		for (i = 0; i < CONST_MAX_UNDO_STEPS - 1; i++) // reshifting all redo elements
			x->r_ob.old_redo_llll[i]=x->r_ob.old_redo_llll[i+1];
		x->r_ob.old_redo_llll[CONST_MAX_UNDO_STEPS - 1] = NULL;
		unlock_general_mutex((t_notation_obj *)x);	
	} else {
		object_warn((t_object *)x, "Can't redo!");
	}
}

void roll_old_undo(t_roll *x){
	if (x->r_ob.old_undo_llll[1]) {
		int i;
		set_roll_from_llll(x, x->r_ob.old_undo_llll[1], true); // resetting roll
		
		lock_general_mutex((t_notation_obj *)x);	
		for (i = CONST_MAX_UNDO_STEPS - 1; i > 0; i--) // reshifting all redo elements
			x->r_ob.old_redo_llll[i]=x->r_ob.old_redo_llll[i-1];
		x->r_ob.old_redo_llll[0] = x->r_ob.old_undo_llll[0];
		for (i = 0; i < CONST_MAX_UNDO_STEPS - 1; i++) // reshifting all undo elements
			x->r_ob.old_undo_llll[i]=x->r_ob.old_undo_llll[i+1];
		x->r_ob.old_undo_llll[CONST_MAX_UNDO_STEPS - 1] = NULL;
		unlock_general_mutex((t_notation_obj *)x);	
	} else {
		object_warn((t_object *)x, "Can't undo!");
	}
}


// what = -1 -> undo, what = 1 -> redo
void roll_new_undo_redo(t_roll *x, char what){
	t_llll *llll = NULL;
	long undo_op = k_UNDO_OP_UNKNOWN;
	char need_check_order = 0;
	
	lock_general_mutex((t_notation_obj *)x);	
	systhread_mutex_lock(x->r_ob.c_undo_mutex);	

	if (what == k_UNDO)
		llll = x->r_ob.undo_llll;
	else if (what == k_REDO)
		llll = x->r_ob.redo_llll;
	
	if (!llll) {
		systhread_mutex_unlock(x->r_ob.c_undo_mutex);	
		unlock_general_mutex((t_notation_obj *)x);	
		return;
	}
	
	while (llll->l_head && hatom_gettype(&llll->l_head->l_hatom) != H_LONG){
		object_error((t_object *) x, what == k_UNDO ? "Wrongly placed undo tick!" : "Wrongly placed redo tick!");
		llll_destroyelem(llll->l_head);
	}

	if (!llll->l_head) {
		if (!(atom_gettype(&x->r_ob.max_undo_steps) == A_LONG && atom_getlong(&x->r_ob.max_undo_steps) == 0))
			object_warn((t_object *) x, what == k_UNDO ? "Can't undo!" : "Can't redo!");
		systhread_mutex_unlock(x->r_ob.c_undo_mutex);	
		unlock_general_mutex((t_notation_obj *)x);	
		return;
	}
	
	undo_op = hatom_getlong(&llll->l_head->l_hatom);
	if (x->r_ob.verbose_undo_redo) {
		char *buf = undo_op_to_string(undo_op);
		object_post((t_object *) x, "%s %s", what == k_UNDO ? "Undo" : "Redo", buf);
		bach_freeptr(buf);
	}
	
	// Destroy the marker
	if (llll->l_head == x->r_ob.last_undo_marker) 
		x->r_ob.last_undo_marker = NULL;
	llll_destroyelem(llll->l_head);
	

	if (what == k_UNDO)
		x->r_ob.num_undo_steps--;
	else
		x->r_ob.num_redo_steps--;
	
	char need_recompute_total_length = false;
	
	while (llll->l_head && hatom_gettype(&llll->l_head->l_hatom) == H_OBJ){
		t_undo_redo_information *this_information = (t_undo_redo_information *)hatom_getobj(&llll->l_head->l_hatom);
		long ID = this_information->n_it_ID;
		e_element_types type = this_information->n_it_type;
		e_undo_modification_types modif_type = this_information->modification_type;
		long voice_num = this_information->voice_num;
		e_header_elems header_info = this_information->header_info;
		t_llll *content = this_information->n_it_content;
		t_llll *newcontent = NULL;
		t_notation_item *item = (t_notation_item *) shashtable_retrieve(x->r_ob.IDtable, ID);
		t_undo_redo_information *new_information = NULL;
		
		if (!item && modif_type != k_UNDO_MODIFICATION_ADD && type != k_WHOLE_NOTATION_OBJECT && type != k_HEADER_DATA) {
			object_error((t_object *) x, "Wrong undo/redo data");
			llll_destroyelem(llll->l_head);
			continue;
		}
		
		if (modif_type == k_UNDO_MODIFICATION_CHANGE_FLAG) {
			newcontent = get_multiple_flags_for_undo((t_notation_obj *)x, item);
			new_information = build_undo_redo_information(ID, type, k_UNDO_MODIFICATION_CHANGE_FLAG, voice_num, 0, k_HEADER_NONE, newcontent);
			set_multiple_flags_from_llll_for_undo((t_notation_obj *)x, content, item);
			x->r_ob.are_there_solos = are_there_solos((t_notation_obj *)x);
		
		} else if (modif_type == k_UNDO_MODIFICATION_CHANGE_NAME) {
			newcontent = get_names_as_llll(item, false);
			new_information = build_undo_redo_information(ID, type, k_UNDO_MODIFICATION_CHANGE_NAME, voice_num, 0, k_HEADER_NONE, newcontent);
			notation_item_set_names_from_llll((t_notation_obj *)x, item, content);

		} else if (type == k_WHOLE_NOTATION_OBJECT){
			// need to reconstruct the whole roll
			newcontent = get_roll_values_as_llll(x, k_CONSIDER_FOR_UNDO, k_HEADER_ALL, false, true);
			new_information = build_undo_redo_information(0, k_WHOLE_NOTATION_OBJECT, k_UNDO_MODIFICATION_CHANGE, 0, 0, k_HEADER_NONE, newcontent);
			roll_clear_all(x);
/*			#ifdef BACH_UNDO_DEBUG
			t_llll *temp = llll_get();
			llll_clone(newcontent, temp, 1, WHITENULL_llll, NULL);
			llll_writetxt((t_object *)x, gensym("undo.txt"), temp);
			#endif
*/			set_roll_from_llll(x, content, false);
			
		} else if (type == k_CHORD) {
			if (modif_type == k_UNDO_MODIFICATION_CHANGE || modif_type == k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER) {
				newcontent = get_rollchord_values_as_llll((t_notation_obj *) x, (t_chord *) item, k_CONSIDER_FOR_UNDO);
				new_information = build_undo_redo_information(ID, k_CHORD, modif_type, voice_num, 0, k_HEADER_NONE, newcontent);
				set_rollchord_values_from_llll((t_notation_obj *) x, (t_chord *)item, content, 0, true, false, false);
				
				need_recompute_total_length = true;
				// recompute_total_length((t_notation_obj *) x);
				
				if (modif_type == k_UNDO_MODIFICATION_CHANGE_CHECK_ORDER)
					need_check_order = true;
				
			} else if (modif_type == k_UNDO_MODIFICATION_DELETE) {
				newcontent = get_rollchord_values_as_llll((t_notation_obj *)x, (t_chord *) item, k_CONSIDER_FOR_UNDO);
				new_information = build_undo_redo_information(ID, k_CHORD, k_UNDO_MODIFICATION_ADD, voice_num, 0, k_HEADER_NONE, newcontent);
				need_recompute_total_length = true;
				if (delete_chord_from_voice((t_notation_obj *)x, (t_chord *)item, ((t_chord *)item)->prev, false))
					check_correct_scheduling((t_notation_obj *)x, false);
				
			} else if (modif_type == k_UNDO_MODIFICATION_ADD) { 
				new_information = build_undo_redo_information(ID, k_CHORD, k_UNDO_MODIFICATION_DELETE, voice_num, 0, k_HEADER_NONE, newcontent);
				t_chord *newch = addchord_from_llll(x, content, nth_rollvoice(x, voice_num), false, true);
				if (newch)
					newch->need_recompute_parameters = true;
			}
			
		} else if (type == k_HEADER_DATA) {
			if (modif_type == k_UNDO_MODIFICATION_CHANGE) { 
				newcontent = get_roll_values_as_llll(x, k_CONSIDER_FOR_UNDO, header_info, false, true);
				new_information = build_undo_redo_information(0, k_HEADER_DATA, k_UNDO_MODIFICATION_CHANGE, 0, 0, header_info, newcontent);
				set_roll_from_llll(x, content, false);
			}
		} 
		
		if (new_information)
			create_undo_redo_tick((t_notation_obj *) x, -what, 1, new_information, false);
		
		llll_free(content);
		bach_freeptr(this_information);
		llll_destroyelem(llll->l_head);
	}	
	
	if (need_recompute_total_length)
		recompute_total_length((t_notation_obj *)x);

	create_undo_redo_step_marker((t_notation_obj *) x, -what, 1, undo_op, false);

	systhread_mutex_unlock(x->r_ob.c_undo_mutex);	

	if (need_check_order)
		check_all_chords_order(x);
    
    if (x->r_ob.notation_cursor.voice)
        roll_linear_edit_snap_to_chord(x); // just to resnap to chord

	unlock_general_mutex((t_notation_obj *)x);	
		
	handle_change((t_notation_obj *)x, x->r_ob.send_undo_redo_bang ? k_CHANGED_STANDARD_SEND_BANG : k_CHANGED_STANDARD, k_UNDO_OP_UNKNOWN);
}

void roll_prune_last_undo_step(t_roll *x)
{
	prune_last_undo_step((t_notation_obj *)x, true);
}


void roll_inhibit_undo(t_roll *x, long val)
{
	x->r_ob.inhibited_undo = val;
}

void roll_undo(t_roll *x)
{
	if (USE_NEW_UNDO_SYSTEM)
		roll_new_undo_redo(x, k_UNDO);
	else
		roll_old_undo(x);
}


void roll_redo(t_roll *x)
{
	if (USE_NEW_UNDO_SYSTEM)
		roll_new_undo_redo(x, k_REDO);
	else
		roll_old_redo(x);
}

void roll_declare_bach_attributes(t_roll *x){
	// CHORD ATTRIBUTES
	t_bach_attr_manager *man = x->r_ob.m_inspector.attr_manager;
	DECLARE_BACH_ATTR(man, 1, _llllobj_sym_onset, (char *)"Onset (ms)", k_CHORD, t_chord, onset, k_BACH_ATTR_DOUBLE, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
	bach_attribute_add_functions(get_bach_attribute(man, k_CHORD, _llllobj_sym_onset), NULL, NULL, NULL, (bach_attr_process_fn)check_all_chords_order_and_correct_scheduling_fn, NULL);

	// MARKER ATTRIBUTES
	DECLARE_BACH_ATTR(man, -1, _llllobj_sym_role, (char *)"Role", k_MARKER, t_marker, role, k_BACH_ATTR_CHAR, 1, k_BACH_ATTR_DISPLAY_ENUMINDEX, 0, 0);
	t_symbol *markerroles[6];
	markerroles[0] = gensym("None");
	markerroles[1] = gensym("Tempo");
	markerroles[2] = gensym("Time Signature");
	markerroles[3] = gensym("Measure Barline");
	markerroles[4] = gensym("Measure Division");
    markerroles[5] = gensym("Measure Subdivision");
	bach_attribute_add_enumindex(get_bach_attribute(man, k_MARKER, _llllobj_sym_role), 6, markerroles);

	DECLARE_BACH_ATTR(man, -1, _llllobj_sym_value, (char *)"Value", k_MARKER, t_marker, content, k_BACH_ATTR_LLLL, 1, k_BACH_ATTR_DISPLAY_TEXT, 0, 0);
	bach_attribute_add_functions(get_bach_attribute(man, k_MARKER, _llllobj_sym_value), NULL, NULL, NULL, (bach_attr_process_fn)check_all_chords_order_and_correct_scheduling_fn, NULL);
}



// actually moves the voice at the end, and sort voices
// does not clear the voice!
void roll_delete_voice(t_roll *x, t_rollvoice *voice)
{
	long voicenumber_to_delete = voice->v_ob.number;
	t_rollvoice *tmp_voice = voice->next;
    
	if (x->r_ob.num_voices <= 1) {
		object_error((t_object *)x, "Can't delete voice: object has just one voice");
		return;
	}

    if (notation_item_is_selected((t_notation_obj *)x, (t_notation_item *)voice))
        notation_item_delete_from_selection((t_notation_obj *)x, (t_notation_item *)voice);
    
	shift_voicewise_arrays((t_notation_obj *)x, voice->v_ob.number + 1, voice->v_ob.number, x->r_ob.num_voices - voice->v_ob.number - 1);

	if (voice->prev) 
		voice->prev->next = voice->next;
	if (voice->next)
		voice->next->prev = voice->prev;
	if (voice != x->lastvoice) {
		voice->prev = x->lastvoice;
		x->lastvoice->next = voice;
	}
	
	if (voice == x->firstvoice){
		x->r_ob.firstvoice = (t_voice *)voice->next;
		x->firstvoice = voice->next;
	}
	voice->next = NULL;
	
	// shifting voice numbers
	for (; tmp_voice; tmp_voice = tmp_voice->next)
		tmp_voice->v_ob.number--;
	voice->v_ob.number = CONST_MAX_VOICES - 1;

	x->r_ob.lastvoice = (t_voice *)voice;
	x->lastvoice = voice;
	x->r_ob.num_voices--;
	x->r_ob.num_voices_plus_one--;
	
	// handling staff lines
	t_llllelem *elem = llll_getindex(x->r_ob.stafflines_as_llll, voicenumber_to_delete + 1, I_MODULO);
	llll_destroyelem(elem);
	set_stafflines_from_llll((t_notation_obj *)x, x->r_ob.stafflines_as_llll, false); // resync

	// handling voice names
	elem = llll_getindex(x->r_ob.voicenames_as_llll, voicenumber_to_delete + 1, I_MODULO);
	llll_destroyelem(elem);
	set_voicenames_from_llll((t_notation_obj *)x, x->r_ob.voicenames_as_llll, false); // resync
    
    update_solos((t_notation_obj *)x);

	
	if (x->r_ob.link_vzoom_to_height)
		auto_set_rectangle_size((t_notation_obj *) x);
	else
		calculate_voice_offsets((t_notation_obj *) x);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	
}

void roll_delete_voiceensemble(t_roll *x, t_voice *any_voice_in_voice_ensemble)
{
    t_voice *first = voiceensemble_get_firstvoice((t_notation_obj *)x, any_voice_in_voice_ensemble);
    t_voice *last = voiceensemble_get_lastvoice((t_notation_obj *)x, any_voice_in_voice_ensemble);
    if (first == last)
        roll_delete_voice(x, (t_rollvoice *)first);
    else {
        long first_num = first->number;
        long last_num = last->number;
        long i;
        for (i = first_num; i <= last_num; i++) {
            t_rollvoice *temp = nth_rollvoice(x, first_num);
            if (temp)
                roll_delete_voice(x, (t_rollvoice *)temp);
        }
    }
}



void roll_swap_voices(t_roll *x, t_rollvoice *v1, t_rollvoice *v2)
{
	if (!v1 || !v2)
		return;
	
	long idx1 = v1->v_ob.number;
	long idx2 = v2->v_ob.number;
	
	lock_general_mutex((t_notation_obj *)x);

	if (x->firstvoice == v1)
		x->firstvoice = v2;
	else if (x->firstvoice == v2)
		x->firstvoice = v1;
	
	if (x->lastvoice == v1)
		x->lastvoice = v2;
	else if (x->lastvoice == v2)
		x->lastvoice = v1;
	
	x->r_ob.firstvoice = (t_voice *)x->firstvoice;
	x->r_ob.lastvoice = (t_voice *)x->lastvoice;
	
	t_rollvoice *v1prev = v1->prev, *v1next = v1->next, *v2prev = v2->prev, *v2next = v2->next;
	t_rollvoice *temp_prev = (v2->prev == v1 ? v2 : v2->prev), *temp_next = (v2->next == v1 ? v2 : v2->next);
	long temp_num = v2->v_ob.number;
	
	if (v1prev && v1prev != v2) v1prev->next = v2;
	if (v1next && v1next != v2) v1next->prev = v2;
	if (v2prev && v2prev != v1) v2prev->next = v1;
	if (v2next && v2next != v1) v2next->prev = v1;
	
	v2->prev = (v1->prev == v2 ? v1 : v1->prev);
	v2->next = (v1->next == v2 ? v1 : v1->next);
	v2->v_ob.number = v1->v_ob.number;
	
	v1->prev = temp_prev;
	v1->next = temp_next;
	v1->v_ob.number = temp_num;

	swapelem_voicewise_arrays((t_notation_obj *)x, idx1, idx2);
	
	// handling staff lines
	t_llllelem *elem1, *elem2;
	
	long val = llll_check(x->r_ob.stafflines_as_llll);
	elem1 = llll_getindex(x->r_ob.stafflines_as_llll, idx1 + 1, I_STANDARD);
	elem2 = llll_getindex(x->r_ob.stafflines_as_llll, idx2 + 1, I_STANDARD);
	llll_swapelems(elem1, elem2);
	val = llll_check(x->r_ob.stafflines_as_llll);
	set_stafflines_from_llll((t_notation_obj *)x, x->r_ob.stafflines_as_llll, false);
	
	// handling voicenames
	elem1 = llll_getindex(x->r_ob.voicenames_as_llll, idx1 + 1, I_STANDARD);
	elem2 = llll_getindex(x->r_ob.voicenames_as_llll, idx2 + 1, I_STANDARD);
	llll_swapelems(elem1, elem2);
	set_voicenames_from_llll((t_notation_obj *)x, x->r_ob.voicenames_as_llll, false);
	
	unlock_general_mutex((t_notation_obj *)x);

	if (x->r_ob.link_vzoom_to_height)
		auto_set_rectangle_size((t_notation_obj *) x);
	else
		calculate_voice_offsets((t_notation_obj *) x);
	invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	
}

void roll_swap_voiceensembles(t_roll *x, t_rollvoice *v1, t_rollvoice *v2)
{
    t_notation_obj *r_ob = (t_notation_obj *)x;
    t_voice *first1 = voiceensemble_get_firstvoice(r_ob, (t_voice *)v1);
    t_voice *last1 = voiceensemble_get_lastvoice(r_ob, (t_voice *)v1);
    t_voice *first2 = voiceensemble_get_firstvoice(r_ob, (t_voice *)v2);
    t_voice *last2 = voiceensemble_get_lastvoice(r_ob, (t_voice *)v2);
    
    if (first1 == first2 || last1 == last2)
        return; // nothing to do
    
    // suppose we have A B C    D E
    // we have to swap : D <-> C,    A B D    C E
    // we have to swap : D <-> B,    A D B    C E
    // we have to swap : D <-> A,    D A B    C E
    // then:
    // we have to swap : E <-> C,    D A B    C E
    // we have to swap : E <-> B,    A D B    C E
    // we have to swap : E <-> A,    D A B    C E
    
    t_voice *temp1, *temp2;
    for (temp2 = first2; temp2 && temp2->number < r_ob->num_voices; temp2 = get_next_voice(r_ob, temp2)) {
        for (temp1 = first1; temp1 && temp1->number < r_ob->num_voices; temp1 = get_next_voice(r_ob, temp1)) {
            roll_swap_voices(x, (t_rollvoice *)temp1, (t_rollvoice *)temp2);
            if (temp1 == last1)
                break;
        }
        
        if (temp2 == last2)
            break;
    }
}

// moves the last voice in a given point.
// idx_of_the_stafflist_element_in_llll is 1-based
void roll_move_and_reinitialize_last_voice(t_roll *x, t_rollvoice *after_this_voice, t_symbol *key, long clef, t_llll *voicename, long midichannel, long idx_of_the_stafflist_element_in_llll)
{
	t_rollvoice *tmp_voice;
	t_llllelem *elem, *nth;
	
	if (x->r_ob.num_voices < CONST_MAX_VOICES){
		// reinsert last voice after the voice *after_this_voice
		t_rollvoice *voice_to_move = x->lastvoice;
		if (after_this_voice && ((t_rollvoice *)after_this_voice)->next) {
			// insert between two voices
			x->lastvoice->prev->next = NULL;
			x->lastvoice = voice_to_move->prev;
			x->r_ob.lastvoice = (t_voice *)x->lastvoice;
			voice_to_move->next = after_this_voice->next;
			voice_to_move->prev = after_this_voice;
			after_this_voice->next->prev = voice_to_move;
			after_this_voice->next = voice_to_move;
			voice_to_move->v_ob.number = after_this_voice->v_ob.number + 1;
			for (tmp_voice = voice_to_move->next; tmp_voice; tmp_voice = tmp_voice->next)
				tmp_voice->v_ob.number ++;
			shift_voicewise_arrays((t_notation_obj *)x, after_this_voice->v_ob.number + 1, after_this_voice->v_ob.number + 2, x->r_ob.num_voices - after_this_voice->v_ob.number - 1);
		} else if (after_this_voice) {
			// insert at the end. Nothing to do: last voice remains last voice!
		} else {
			// insert at the beginning 
			x->lastvoice->prev->next = NULL;
			x->lastvoice = voice_to_move->prev;
			x->r_ob.lastvoice = (t_voice *)x->lastvoice;
			voice_to_move->next = x->firstvoice;
			voice_to_move->prev = NULL;
			x->firstvoice->prev = voice_to_move;
			x->firstvoice = voice_to_move;
			x->r_ob.firstvoice = (t_voice *)x->firstvoice;
			voice_to_move->v_ob.number = 0;
			for (tmp_voice = voice_to_move->next; tmp_voice; tmp_voice = tmp_voice->next)
				tmp_voice->v_ob.number ++;
			shift_voicewise_arrays((t_notation_obj *)x, 0, 1, x->r_ob.num_voices);
		}
		x->r_ob.num_voices++;
		x->r_ob.num_voices_plus_one++;

		long voice_num = CLAMP(voice_to_move->v_ob.number, 0, CONST_MAX_VOICES - 1);
		x->r_ob.hidevoices_as_charlist[voice_num] = 0;
		x->r_ob.voiceuspacing_as_floatlist[voice_num] = CONST_DEFAULT_ROLLVOICES_SPACING_UY;
		x->r_ob.show_measure_numbers[voice_num] = 1;

		// handling staff lines
		elem = llll_getindex(x->r_ob.stafflines_as_llll, idx_of_the_stafflist_element_in_llll, I_MODULO);
		nth = llll_getindex(x->r_ob.stafflines_as_llll, voice_to_move->v_ob.number, I_MODULO);
		if (voice_to_move->v_ob.number > 0)
			llll_inserthatom_after_clone(&elem->l_hatom, nth, 0, WHITENULL_llll);
		else
			llll_prependhatom_clone(x->r_ob.stafflines_as_llll, &elem->l_hatom, 0, WHITENULL_llll);
		set_stafflines_from_llll((t_notation_obj *)x, x->r_ob.stafflines_as_llll, false);

		// handling voicenames
		nth = llll_getindex(x->r_ob.voicenames_as_llll, voice_to_move->v_ob.number, I_MODULO);
		if (voice_to_move->v_ob.number > 0)
			llll_insertllll_after(voicename, nth, 0, WHITENULL_llll);
		else
			llll_prependllll(x->r_ob.voicenames_as_llll, voicename, 0, WHITENULL_llll);
		set_voicenames_from_llll((t_notation_obj *)x, x->r_ob.voicenames_as_llll, false);

		change_single_key((t_notation_obj *)x, (t_voice *)voice_to_move, key, false);
		change_single_clef((t_notation_obj *)x, (t_voice *)voice_to_move, clef, false);
		voice_to_move->v_ob.midichannel = midichannel;
		voice_to_move->v_ob.locked = voice_to_move->v_ob.solo = voice_to_move->v_ob.hidden = voice_to_move->v_ob.muted = 0;
        
		if (x->r_ob.link_vzoom_to_height)
			auto_set_rectangle_size((t_notation_obj *) x);
		else
			calculate_voice_offsets((t_notation_obj *) x);
		invalidate_notation_static_layer_and_repaint((t_notation_obj *) x);
	}
}

void roll_resetslotinfo(t_roll *x)
{
	create_whole_roll_undo_tick(x);
	notation_obj_reset_slotinfo((t_notation_obj *)x);
	handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_SLOTINFO);
}


void roll_resetarticulationinfo(t_roll *x)
{
    create_whole_roll_undo_tick(x);
    notation_obj_reset_articulationinfo((t_notation_obj *)x);
    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_CUSTOM_ARTICULATIONS_DEFINITION);
}

void roll_resetnoteheadinfo(t_roll *x)
{
    create_whole_roll_undo_tick(x);
    notation_obj_reset_noteheadinfo((t_notation_obj *)x);
    handle_change_if_there_are_free_undo_ticks((t_notation_obj *) x, k_CHANGED_STANDARD_UNDO_MARKER, k_UNDO_OP_CHANGE_CUSTOM_NOTEHEADS_DEFINITION);
}


void roll_mute(t_roll *x)
{
	mute_selection((t_notation_obj *)x, false);
}

void roll_unmute(t_roll *x)
{
	unmute_selection((t_notation_obj *)x, false);
}

void roll_lock(t_roll *x)
{
	lock_selection((t_notation_obj *)x, false);
}

void roll_unlock(t_roll *x)
{
	unlock_selection((t_notation_obj *)x, false);
}

void roll_solo(t_roll *x)
{
	solo_selection((t_notation_obj *)x, false);
}

void roll_unsolo(t_roll *x)
{
	unsolo_selection((t_notation_obj *)x, false);
}

void roll_copy_fn(t_notation_obj *r_ob, t_note *nt, void *dummy){
    long slotnum = *((long *)dummy);
    notation_obj_copy_slot(r_ob, &clipboard, (t_notation_item *)nt, slotnum, false);
}

void score_cut_fn(t_notation_obj *r_ob, t_note *nt, void *dummy){
    long slotnum = *((long *)dummy);
    notation_obj_copy_slot(r_ob, &clipboard, (t_notation_item *)nt, slotnum, true);
}

void roll_copyslot_fn(t_notation_obj *r_ob, t_note *nt, void *dummy){
    long slotnum = *((long *)dummy);
    notation_obj_copy_slot(r_ob, &clipboard, (t_notation_item *)nt, slotnum, false);
}

void roll_cutslot_fn(t_notation_obj *r_ob, t_note *nt, void *dummy){
    long slotnum = *((long *)dummy);
    notation_obj_copy_slot(r_ob, &clipboard, (t_notation_item *)nt, slotnum, true);
}

void roll_copy_or_cut(t_roll *x, t_symbol *s, long argc, t_atom *argv, char cut){
	t_llll *ll = llllobj_parse_llll((t_object *)x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_RETAIN);
    if (ll->l_size >= 1 && hatom_gettype(&ll->l_head->l_hatom) == H_SYM && (hatom_getsym(&ll->l_head->l_hatom) == gensym("durationline"))) {
        t_note *note = get_leftmost_selected_note((t_notation_obj *)x);
        notation_obj_copy_durationline((t_notation_obj *)x, &clipboard, note, cut);
    } else if (ll->l_size >= 1 && hatom_gettype(&ll->l_head->l_hatom) == H_SYM && (hatom_getsym(&ll->l_head->l_hatom) == _llllobj_sym_slot || hatom_getsym(&ll->l_head->l_hatom) == _llllobj_sym_slots)) {
        long slotnum = ll->l_size >= 2 && hatom_gettype(&ll->l_head->l_next->l_hatom) == H_LONG ? hatom_getlong(&ll->l_head->l_next->l_hatom) - 1 : (ll->l_size >= 2 && hatom_gettype(&ll->l_head->l_next->l_hatom) == H_SYM && hatom_getsym(&ll->l_head->l_next->l_hatom) == _sym_all ? -1 : x->r_ob.active_slot_num);
		if (x->r_ob.active_slot_notationitem)
			notation_obj_copy_slot((t_notation_obj *)x, &clipboard, x->r_ob.active_slot_notationitem, slotnum, cut);
        else
            iterate_notewise_changes_on_selection((t_notation_obj *)x, cut ? (notation_obj_note_fn)roll_cutslot_fn : (notation_obj_note_fn)roll_copyslot_fn, &slotnum, true, k_CHORD, false);
	} else {
		roll_copy_selection(x, cut);
	}
	llll_free(ll);
}


void roll_copy(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	roll_copy_or_cut(x, s, argc, argv, false);
}

void roll_cut(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	roll_copy_or_cut(x, s, argc, argv, true);
}

void roll_paste(t_roll *x, t_symbol *s, long argc, t_atom *argv)
{
	t_llll *ll = llllobj_parse_llll((t_object *)x, LLLL_OBJ_UI, NULL, argc, argv, LLLL_PARSE_RETAIN);
	
	if (ll->l_size >= 1 && hatom_gettype(&ll->l_head->l_hatom) == H_SYM && (hatom_getsym(&ll->l_head->l_hatom) == gensym("durationline"))) {
		if (clipboard.type == k_DURATION_LINE)
            notation_obj_paste_durationline((t_notation_obj *) x, &clipboard);
    } else if (ll->l_size >= 1 && hatom_gettype(&ll->l_head->l_hatom) == H_SYM && (hatom_getsym(&ll->l_head->l_hatom) == _llllobj_sym_slot || hatom_getsym(&ll->l_head->l_hatom) == _llllobj_sym_slots)) {
            if (clipboard.type == k_SLOT)
                notation_obj_paste_slot((t_notation_obj *) x, &clipboard,
                                        ll->l_size >= 2 && hatom_gettype(&ll->l_head->l_next->l_hatom) == H_LONG ? hatom_getlong(&ll->l_head->l_next->l_hatom) - 1 :
                                        (ll->l_size >= 2 && hatom_gettype(&ll->l_head->l_next->l_hatom) == H_SYM &&
                                         hatom_getsym(&ll->l_head->l_next->l_hatom) == _llllobj_sym_active ? x->r_ob.active_slot_num : -1));
	} else {
		if (clipboard.type == k_SELECTION_CONTENT) {
			if (clipboard.object == k_NOTATION_OBJECT_ROLL && clipboard.gathered_syntax && clipboard.gathered_syntax->l_head) {
                long how_many_times = 1;
                llll_parseargs_and_attrs_destructive((t_object *) x, ll, "i", gensym("repeat"), &how_many_times);
                
                for (long i = 0; i < how_many_times; i++) {
                    char inplace = true, inplacevoice = true;
                    double start_ms = 0;
                    long start_voice = 1;
                    if (ll->l_head) {
                        if (is_hatom_number(&ll->l_head->l_hatom)) {
                            start_ms = hatom_getdouble(&ll->l_head->l_hatom);
                            inplace = false;
                        } else if (hatom_gettype(&ll->l_head->l_hatom) == H_SYM && hatom_getsym(&ll->l_head->l_hatom) == _llllobj_sym_end) {
                            start_ms = x->r_ob.length_ms_till_last_note;
                            inplace = false;
                        } else if (hatom_gettype(&ll->l_head->l_hatom) == H_SYM && hatom_getsym(&ll->l_head->l_hatom) == _sym_replace) {
                            // paste replace!!!
                            start_ms = get_selection_leftmost_onset(x);
                            start_voice = get_selection_topmost_voice(x);
                            roll_delete_selection(x, false);
                            inplace = false;
                        }
                        if (ll->l_head->l_next) {
                            if (is_hatom_number(&ll->l_head->l_next->l_hatom)) {
                                start_voice = hatom_getlong(&ll->l_head->l_next->l_hatom);
                                inplacevoice = false;
                            }
                        }
                    }
                    roll_paste_clipboard(x, inplace, start_ms, inplacevoice, start_voice - 1, false, i == 0);
                }
			}
		}
	}
	
	llll_free(ll);
}