;============================================================================;
; utilities.tf (formerly mylib.tf)                                           ;
; This contains common functions that were missing in the main tf library    ;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;
;;; Virtual Array 
;;;
;;; Originally submitted to the Tinyfugue mailing list]
;;; by Galvin <kairo at mediaone dot net>
;;; Modified by Michael Hunger <mh14 at inf.tu-dresden dot de>
;;; Modified by Regan@ Many MU's
;;; Possibly modified by other members of the Tinyfugue mailing list.
;;;
;;; You may store anything to be abbreviated in
;;; these lists, e.g. weapons, friends, npcs, scores etc.
;;;
;;; If you use text not containing other characters than letters, numbers and _
;;; you may also use this as an simple key-value hash list
;;; e.g. /set_array weapon s1 sword_with_a_very_long_id
;;;
;;; /def wield = /get_array weapon %1%;wield %?
;;;
;;; Usage:
;;; instead of: wield sword_with_a_very_long_id
;;; just use: /wield s1
;;;
;;; Some Examples:
;;; /test put_array('test_array', 1, 367382)
;;; /test put_array('test_array', 2, 'a string')
;;; /test echo(get_array('test_array', 1))
;;; /test echo(get_array('test_array', 2))
;;; 
;;; You should see 367382 and a string echoed to your screen.
;;; 
;;; Passing an array to a function
;;; 
;;; /def function_array = \
;;; /test array1 := get_array({*}, 1) %;\
;;; /test array2 := get_array({*}, 2) %;\
;;; /test echo(strcat(array1, ':', array2))
;;; 
;;; /test function_array('test_array')
;;; 
;;; Debugging can be done with /listarray which would return:
;;; /listarray test_array
;;; test_array[1]:=367382
;;; test_array[2]:=a string
;;; test_array[3]:=
;;;
;;; /listarray only lists up till a blank entry,
;;; I thought about using entry 0 as the # of elements but decided 
;;; that some people may want to start arrays from 0 and not 1.
;;;
;;; Some get_array_count examples:
;;;
;;; /test put_array('halloween', 1, 'skeleton')
;;; /test put_array('halloween', 2, 'owls')
;;; /test put_array('halloween', 3, 'boo!')
;;; 
;;; /test echo(get_array_count('halloween', 1))
;;; returns 3
;;; /test echo(get_array_count('halloween', 2))
;;; returns 2
;;; 
;;; /test put_2array('double', 1, 1 , 'hmm')
;;; /test put_2array('double, 1, 2, 'burp!')
;;; /test put_2array('double, 1, 3, 'damn')
;;; /test echo(get_2array_count('double', 1, 1) 
;;;
;----------------------------------------------------------------------------;

/loaded __TFLIB__/utilities.tf

;-----------------------------------------------------------------------------
;Get array / get 2array : A virtual array function to similate a real array
;usage:
;get_array("Array name here", I) & get_2array("array name", I, I2)
;example get_array("Dir_Array", 43) returns the 43rd element from "Dir_Stack"
;-----------------------------------------------------------------------------
/def get_array = \
 /return _array_%1_%2

/def get_2array = \
 /return _array_%1_%2_%3

;-----------------------------------------------------------------------------
;PUT array / put 2array: A virtual array function to similate a real array
;usage:
;put_array("Array name here", I, st) & put_2array("array name", I, I2, st)
;example put_array("Dir_Array", 43, "sw") puts "sw" at element 43 in
;"Dir_Array"
;-----------------------------------------------------------------------------
/def put_array = \
 /IF (strlen({3}) > 0) \
   /set _array_%1_%2=%3%;\
 /ELSE \
   /unset _array_%1_%2%;\
 /ENDIF%;\

/def put_2array = \
 /IF (strlen({4}) > 0) \
   /set _array_%1_%2_%3=%4%;\
 /ELSE \
   /unset _array_%1_%2_%3%;\
 /ENDIF%;\

;-----------------------------------------------------------------------------
;PURGE array : Purges a virtual array made by get_array & put_array
;usage:
;purge_array("Array name here")
;example purge_array("Dir_Array"), deletes the whole array from memory
;NOTE: Purge array starts from element 0
;NOTE: this can also purge double dimensioned arrays too.
;-----------------------------------------------------------------------------
/def purge_array = \
 /quote -S /unset `/listvar -s _array_%1_*

;-----------------------------------------------------------------------------
;listarray / list2array
;USAGE:
;/listarray array_name <num> & /list2array array_name <num> <num2>
;Will list the whole array of array_name starting from element <num>
;/list2array only lists the second dimension from <start>
;-----------------------------------------------------------------------------
/def listarray = \
 /test LA_Count := %2 - 1%;\
 /test LA_Element := " "%;\
 /while (strlen(LA_Element) > 0) \
   /test ++LA_Count%;\
   /test LA_Element := get_array({1}, LA_Count)%;\
   /test echo(strcat({1}, "[", LA_Count, "]:=", LA_Element))%;\
 /DONE

/def list2array = \
 /test LA2_Count := -1%;\
 /test LA2_Element := " "%;\
 /while (LA2_Count < 255) \
   /test ++LA2_Count%;\
   /test LA2_Element := get_2array({1}, {2}, LA2_Count)%;\
   /IF (strlen(LA2_Element) > 0) \
     /test echo(strcat({1}, "[", {2}, "][", LA2_Count, "]:=", LA2_Element))%;\
   /ENDIF%;\
 /DONE

;-----------------------------------------------------------------------------
; list_array() / list_2array()
;
; List an array in an easy to read format.
; USAGE:
; list_array("array_name", startindex)
; list_2array("array_name", startindex1, startindex2)
; /list_array <array_name> <startindex>
; /list_2array <array_name> <startindex1> <startindex2>
;
; "array_name" : Name of the array you want to list. (function form only)
; startindex   : Starting element you want to list from.
; startindex1  : Starting element of startindex2 you want to list from.
; startindex2  : Starting element of startindex1
;-----------------------------------------------------------------------------
/def list_array = \
  /def list_array2 = \
    /test la_rawname := {1} %%;\
    /test la_name := {2} %%;\
    /test la_index := substr(la_rawname, strlen(la_name) + 8, 256) %%;\
    /IF (la_index >= {3}) \
      /test echo(strcat(la_name, '[', la_index, '] :=', get_array(la_name, la_index))) %%;\
    /ENDIF %;\
  /quote -S /test list_array2("`"/listvar -s _%1_array_*"", {1}, {2}) %;\
  /undef list_array2

/def list_2array = \
  /test la2_i2F := 0 %;\
  /def list_2array2 = \
    /test la2_rawname := {1} %%;\
    /test la2_name := {2} %%;\
    /test la2_rawindex := substr(la2_rawname, strlen(la2_name) + 8, 256) %%;\
    /test la2_pos := strstr(la2_rawindex, '_') %%;\
    /test la2_index1 := substr(la2_rawindex, 0, la2_pos) %%;\
    /test la2_index2 := substr(la2_rawindex, la2_pos + 1, 255) %%;\
    /IF (la2_index1 >= {3}) \
      /IF ( (la2_i2F = 0) & (la2_index2 >= {4}) )\
        /test la2_i2F := 1 %%;\
      /ENDIF %%;\
      /IF (la2_i2F = 1) \
        /test echo(strcat(la2_name, '[', la2_index1, '][', la2_index2, '] :=', get_2array(la2_name, la2_index1, la2_index2))) %%;\
      /ENDIF %%;\
    /ENDIF %;\
  /quote -S /test list_2array2("`"/listvar -s _%1_array_*"", {1}, {2}, {3}) %;\
  /undef list_2array2

;-----------------------------------------------------------------------------
;GET array count / GET 2array count
; Written by: Ian Leisk who may actually be "Galvin", but,
; may not (kairo at attbi dot com)!
;usage:
;get_array_count("Array name here", start)
;get_2array_count("Array name here", index, start)
;
;NOTE:
;These will count the number of elements starting at "start" till the first
;empty element.
;Get_array2_count will count the number of elements starting at index
;from "start"
;-----------------------------------------------------------------------------

/def get_array_count = \
  /test GA_Name := {1} %;\
  /test GA_Count := {2} - 1 %;\
  /test GA_Element := " " %;\
  /while (strlen(GA_Element) > 0) \
    /test ++GA_Count %;\
    /test GA_Element := get_array(GA_Name, GA_Count) %;\
  /DONE %;\
  /return GA_Count - 1

/def get_2array_count = \
  /test GA2_Name := {1} %;\
  /test GA2_Index := {2} %;\
  /test GA2_Count := {3} -1 %;\
  /test GA2_Element := " " %;\
  /while (strlen(GA2_Element) > 0) \
     /test ++GA2_Count %;\
     /test GA2_Element := get_2array(GA2_Name, GA2_Index, GA2_Count) %;\
  /DONE %;\
  /return GA2_Count - 1

;-----------------------------------------------------------------------------
; tfwrite_array() / tfwrite_2array()
;
; Writes a virtual array to a disk file.
; USAGE:
; tfwrite_array(file_variable, "array_name", start_index, size)
; tfwrite_2array(file_variable, "array_name", index, start_index, size)
;
; file_variable : File variable of a tfopened file.
; "array_name"  : Name of the array.
; start_index   : The element you want to start writing from.
; index         : Start_index of index [first dimension] (tfwrite_2array only)
; Size          : Number of elements to write.
;
; NOTES: tfwrite_2array() can't write the whole array to disk.  You must
;        write each dimension at a time.
;-----------------------------------------------------------------------------
/def tfwrite_array = \
  /let file_ %{1} %;\
  /let array_ %{2} %;\
  /let start_ %{3} %;\
  /let size_ %{4} %;\
  /let count_ 0 %;\
  /WHILE (++count_ <= size_) \
    /test tfwrite(file_, get_array(array_, start_)) %;\
    /test ++start_ %;\
  /DONE

/def tfwrite_2array = \
  /let file_ %{1} %;\
  /let array_ %{2} %;\
  /let index_ %{3} %;\
  /let start_ %{4} %;\
  /let size_ %{5} %;\
  /let count_ 0 %;\
  /WHILE (++count_ <= size_) \
    /test tfwrite(file_, get_2array(array_, index_, start_)) %;\
    /test ++start_ %;\
  /DONE

;-----------------------------------------------------------------------------
; tfread_array() / tfread_2array()
;
; reads an array file from disk
; USAGE:
; x := tfread_array(file_variable, "array_name", start_index, size)
; x := tfread_2array(file_variable, "array_name", index, start_index, size)
;
; x             : Number of records read.
; file_variable : File variable of a tfopened file.
; "array_name"  : Name of the array.
; start_index   : Starting element you want to read into.
; index         : Start_index of index [first dimension] (tfread_2array only)
; size          : Number of elements to read.
;-----------------------------------------------------------------------------
/def tfread_array = \
  /let file_ %{1} %;\
  /let array_ %{2} %;\
  /let start_ %{3} %;\
  /let size_ %{4} %;\
  /let count_ 0 %;\
  /let done_ 0 %;\
  /let err_ 0 %;\
  /let st_ 0 %;\
  /test st_ := '' %;\
  /WHILE (!done_) \
    /test err_ := tfread(file_, st_) %;\
    /IF ( (err_ != -1) & (count_ < size_) ) \
      /test put_array(array_, start_, st_) %;\
      /test ++count_ %;\
      /test ++start_ %;\
    /ELSE \
      /test done_ := 1 %;\
    /ENDIF %;\
  /DONE %;\
  /RETURN count_

/def tfread_2array = \
  /let file_ %{1} %;\
  /let array_ %{2} %;\
  /let index_ %{3} %;\
  /let start_ %{4} %;\
  /let size_ %{5} %;\
  /let count_ 0 %;\
  /let done_ 0 %;\
  /let err_ 0 %;\
  /let st_ 0 %;\
  /test st_ := '' %;\
  /WHILE (!done_) \
    /test err_ := tfread(file_, st_) %;\
    /IF ( (err_ != -1) & (count_ < size_) ) \
      /test put_2array(array_, index_, start_, st_) %;\
      /test ++count_ %;\
      /test ++start_ %;\
    /ELSE \
      /test done_ := 1 %;\
    /ENDIF %;\
  /DONE %;\
  /RETURN count_

;-----------------------------------------------------------------------------
; strstr_array() / strstr_2array()
;
; Searches for a value in a virtual array returning the element its found in.
; USAGE:
; x := strstr_array("array_name", start_index, size, value)
; x := strstr_2array("array_name", index, start_index, size, value)
;
; x             : Element of "array_name" that value was found in.
;                 -1 is returned if value was not found.
; "array_name"  : Name of the array.
; start_index   : Element to start searching at
; index         : Start_index of index [first dimension] (strstr_2arrat only).
; size          : Number of elements to search.
; value         : The item your searching for.
;
; NOTES: Strstr_2array can't search all dimensions, you must search
;        each dimension at a time.
;        If value = "" then it will return the first element that is blank.
;-----------------------------------------------------------------------------
/def strstr_array = \
  /let array_ %{1} %;\
  /let start_ %{2} %;\
  /let size_ %{3} %;\
  /let value_ 0 %;\
  /test value_ := {4} %;\
  /let count_ 0 %;\
  /let pos_ 0 %;\
  /let st_ 0 %;\
  /test st_ := '' %;\
  /let element_ -1 %;\
  /WHILE ( (++count_ <= size_) & (element_ = -1) ) \
    /test st_ := get_array(array_, start_) %;\
    /IF (value_ =~ '') \
      /IF (st_ =~ '') \
        /test element_ := start_ %;\
      /ENDIF %;\
    /ELSE \
      /test pos_ := strstr(st_, value_) %;\
      /IF (pos_ > -1) \
        /test element_ := start_ %;\
      /ENDIF %;\
    /ENDIF %;\
    /test ++start_ %;\
  /DONE %;\
  /RETURN element_

/def strstr_2array = \
  /let array_ %{1} %;\
  /let index_ %{2} %;\
  /let start_ %{3} %;\
  /let size_ %{4} %;\
  /let value_ 0 %;\
  /test value_ := {5} %;\
  /let count_ 0 %;\
  /let pos_ 0 %;\
  /let st_ 0 %;\
  /let element_ -1 %;\
  /test st_ := '' %;\
  /WHILE ( (++count_ <= size_) & (element_ = -1) ) \
    /test st_ := get_2array(array_, index_, start_) %;\
    /IF (value_ =~ '') \
      /IF (st_ =~ '') \
        /test element_ := start_ %;\
      /ENDIF %;\
    /ELSE \
      /test pos_ := strstr(st_, value_) %;\
      /IF (pos_ > -1) \
        /test element_ := start_ %;\
      /ENDIF %;\
    /ENDIF %;\
    /test ++start_ %;\
  /DONE %;\
  /RETURN element_;

;-----------------------------------------------------------------------------
; searchstr_2array()
;
; Searches for a value in all dimensions of a virtual array and returns
; each element its found in. Multiple matches are put into a space-delimited
; list
; Modified by Regan @ Many MU's
;
; USAGE:
; x := searchstr_2array("array_name", index, start_index, size, value)
;
; x             : Element of "array_name" that value was found in.
;                 -1 is returned if value was not found.
; "array_name"  : Name of the array.
; start_index   : Element to start searching at
; index         : Start_index of index [first dimension] (strstr_2arrat only).
; size          : Number of elements to search.
; value         : The item your searching for.
;
;        If value = "" then it will return the first element that is blank.
;-----------------------------------------------------------------------------

/def searchstr_2array = \
  /let array_ %{1} %;\
  /let index_ %{2} %;\
  /let start_ %{3} %;\
  /let size_ %{4} %;\
  /let value_ 0 %;\
  /test value_ := {5} %;\
  /let count_ 0 %;\
  /let pos_ 0 %;\
  /let st_ 0 %;\
  /let element_ -1 %;\
  /test blank_ := ' ' %;\
  /test list_ := '' %;\
  /test st_ := '' %;\
  /WHILE (++count_ <= size_) \
    /test st_ := get_2array(array_, start_, index_) %;\
    /IF (value_ =~ '') \
      /IF (st_ =~ '') \
        /test element_ := start_ %;\
        /IF (list_ =~ '') \
          /test list_ := element_ %;\
        /ELSE \
          /test temp_ := strcat(list_, blank_) %:\
          /test list_ := strcat(temp_, element_) %;\
        /ENDIF %;\
      /ENDIF %;\
    /ELSE \
      /test pos_ := strstr(st_, value_) %;\
      /IF (pos_ > -1) \
        /test element_ := start_ %;\
        /IF (list_ =~ '') \
          /test list_ := element_ %;\
        /ELSE \
          /test temp_ := strcat(list_, blank_) %;\
          /test list_ := strcat(temp_, element_) %;\
        /ENDIF %;\
      /ENDIF %;\
    /ENDIF %;\
    /test ++start_ %;\
  /DONE %;\
  /RETURN list_

;-----------------------------------------------------------------------------
; chr2chr()
;
; Changes one character to difference character in a string
; USAGE:
; x := chr2chr("string", "searchchar", "changechar")
;
; x           : contains "string" with all "." as "_".
; "string"    : any string.
; "searchar"  : The character to be changed in "string".
; "changechar : The character to replace each occrance of "searchar" in
;               "string"."
;-----------------------------------------------------------------------------
/def chr2chr = \
  /let st_ 0 %;\
  /test st_ := {1} %;\
  /let searchchar_ 0 %;\
  /test searchchar_ := {2} %;\
  /let changechar_ 0 %;\
  /test changechar_ := {3} %;\
  /let pos_ -1 %;\
  /let pos2_ 0 %;\
  /let done_ 0 %;\
  /WHILE (!done_) \
    /test pos2_ := pos_ %;\
    /test pos_ := strchr(substr(st_, pos_ + 1), searchchar_) %;\
    /IF (pos_ > -1) \
      /test pos_ := pos_ + pos2_ + 1 %;\
      /IF (substr(st_, pos_, 1) =~ searchchar_) \
        /test st_ := strcat(substr(st_, 0, pos_), changechar_, substr(st_, pos_ + 1)) %;\
      /ENDIF %;\
    /ELSE \
      /test done_ := 1 %;\
    /ENDIF %;\
  /DONE %;\
  /RETURN st_

;-----------------------------------------------------------------------------
; tfwrite_vars()
; Write variables to a disk file.
; USAGE:
; x := tfwrite_vars("variables", "matching", file_variable)
;
; x             : Number of variables written to disk.
; "variables"   : A string matching the variables you want to write to disk.
; "matching"    : "simple": straightforward string comparison.
;                 "glob"  : shell-like matching (as before version 3.2).
;                 "regexp": regular expression.
;                 ""      : defaults to "glob"
; file_variable : File variable of a tfopened file.
;
; EXAMPLES:
; /set movie_title lord of the rings
; /set movie_year 2001
; /set movie_length 2 hours 59 min
; /test file_ := tfopen("movies.dat", "w")
; /test tfwrite_vars("movie_*", "glob", file_)
; /test tfclose(file_)
;
; This will write all 3 variables to disk.
;
; NOTES: The variables must be global variables made with /set or
;        /test var := value.
;-----------------------------------------------------------------------------
/def tfwrite_vars = \
  /let vars_ %{1} %;\
  /let matching_ 0 %;\
  /test matching_ := {2} %;\
  /IF (matching_ =~ '') \
    /test matching_ := "glob" %;\
  /ENDIF %;\
  /let file_ %{3} %;\
  /let count_ 0 %;\
  /def tfwrite_vars2 = \
    /let var_ %%{1} %%;\
    /test ++count_ %%;\
    /test echo(strcat('::',$$[var_],'::')) %%;\
    /test tfwrite(file_, strcat(':',var_,'=',$$[var_], ':')) %;\
  /quote -S /test tfwrite_vars2("`"/listvar -m$[matching_] -s $[vars_]"") %;\
  /undef tfwrite_vars2 %;\
  /return count_

;-----------------------------------------------------------------------------
; tfread_vars()
; Reads variables to a from a disk file that was written by tfwrite_vars()
; USAGE:
; x := tfread_vars(file_variable)
;
; x             : number of variables read.
; file_variable : File variable of a tfopened file.
;
; EXAMPLES:
;
; /test file_ := tfopen("movies.dat", "r")
; /test tfread_vars(file_)
; /test tfclose(file_)
;
; Now the variables will be set to the same values as shown in the
; tfwrite_vars() examples.
;
; NOTE: The variables do not have to exist prior to reading since they're
;       already written in the file by tfwrite_vars()
;-----------------------------------------------------------------------------
/def tfread_vars = \
  /let file_ %{1} %;\
  /let done_ 0 %;\
  /let st_ 0 %;\
  /let pos_ 0 %;\
  /let var_ 0 %;\
  /let value_ 0 %;\
  /let err_ 0 %;\
  /let count_ 0 %;\
  /WHILE (!done_) \
    /test err_ := tfread(file_, st_) %;\
    /IF (err_ != -1) \
      /test ++count_ %;\
      /test st_ := substr(st_, 1, strlen(st_) - 2) %;\
      /test pos_ := strstr(st_, '=') %;\
      /test var_ := substr(st_, 0, pos_) %;\
      /test value_ := substr(st_, pos_ +1) %;\
      /eval /test $[var_] := value_ %;\
    /ELSE \
      /test done_ := 1 %;\
    /ENDIF %;\
  /DONE %;\
  /RETURN count_

;-----------------------------------------------------------------------------
; delay()
;
; Delays an X number of seconds before returning.
; USAGE:
; delay(seconds)
;-----------------------------------------------------------------------------
/def delay = \
  /let seconds_ %{1} %;\
  /let ctime_ 0 %;\
  /test ctime_ := get_time(0) %;\
  /WHILE ( (get_time(0) - ctime_) < seconds_ ) \
  /DONE

;-----------------------------------------------------------------------------
; get_time()
;
; returns hours, minutes, seconds in integer format
; USAGE:
; x := get_time(0)
; x := get_time(raw_time)
;
; x        : Time in seconds.
; 0        : Current time in seconds returned.
; raw_time : Raw time, raw time gets passed back from tinyfugue's time()
;            This will get converted to seconds.
;-----------------------------------------------------------------------------
/def get_time = \
  /let st_ 0 %;\
  /let hours_ 0 %;\
  /let min_ 0 %;\
  /let sec_ 0 %;\
  /IF ({*} = 0) \
    /test st_ := ftime('%%H:%%M:%%S', time()) %;\
  /ELSE \
    /test st_ := ftime('%%H:%%M:%%S', {*}) %;\
  /ENDIF %;\
  /test hours_ := substr(st_, 0, 2) %;\
  /test min_ := substr(st_, 3, 2) %;\
  /test sec_ := substr(st_, 6, 2) %;\
  /return time2sec(hours_, min_, sec_)

;-----------------------------------------------------------------------------
; time2sec()
;
; converts hours, min, sec to total seconds
; USAGE:
;
; x := time2sec(hours, min, sec)
;
; x      : The total time in seconds of hours, min, sec.
; hours  : Can range from 0 to 23.
; min    : Can range from 0 to 59.
; sec    : Can range from 0 to 59.
;-----------------------------------------------------------------------------
/def time2sec = \
  /let h_ %{1} %;\
  /let m_ %{2} %;\
  /let s_ %{3} %;\
  /return h_ * 3600 + m_ * 60 + s_

;-----------------------------------------------------------------------------
; sec2time()
;
; converts total seconds back into hours, min, sec
; USAGE:
; sec2time("time_rec", seconds)
;
; "time_rec" : Contains the hours, min, seconds passed back.
;              get_rec("time_rec", "hours") = hours.
;              get_rec("time_rec", "min") = minutes.
;              get_rec("time_rec", "sec") = seconds.
; seconds    : Value in seconds.
;
; NOTES: "time_rec" can be any record name you want.
;        If "" is specified for "time_rec" then no record gets used. See how
;        this is used in sec2clock.  Local vars are set in calling function
;        to get back the values from this function.
;-----------------------------------------------------------------------------
/def sec2time = \
  /let rec_ 0 %;\
  /test rec_ := {1} %;\
  /let int_ %{2} %;\
  /IF (rec_ !~ '') \
    /let hours_ 0 %;\
    /let min_ 0 %;\
    /let sec_ 0 %;\
  /ENDIF %;\
  /test hours_ := int_ / 3600 %;\
  /let rem_ $[int_ - (hours_ * 3600)] %;\
  /test min_ := rem_ / 60 %;\
  /test rem_ := rem_ - (min_ * 60) %;\
  /test sec_ := rem_ %;\
  /IF (rec_ !~ '') \
    /test del_rec(rec_, '') %;\
    /test add_rec(rec_, 'hours', hours_) %;\
    /test add_rec(rec_, 'min', min_) %;\
    /test add_rec(rec_, 'sec', sec_) %;\
  /ENDIF

;-----------------------------------------------------------------------------
; sec2clock()
;
; similar to sec2time except it turns seconds into a clock reading AM/PM
; USAGE:
; x := sec2clock(value)
;
; x     : Time returned formatted in ##:##pm.  Example : "3:04am"
; value : Total seconds of time ranging over a 24 hour period.
;-----------------------------------------------------------------------------
/def sec2clock = \
  /let rawtime_ %{1} %;\
  /let hours_ 0 %;\
  /let min_ 0 %;\
  /let sec_ 0 %;\
  /let pm_ 0 %;\
  /let st_ 0 %;\
  /test sec2time('', rawtime_) %;\
  /IF (min_ <= 9) \
    /test min_ := strcat('0', min_) %;\
  /ENDIF %;\
  /IF (rawtime_ > 12 * 3600) \
    /test pm_ := 1 %;\
    /test hours_ := hours_ - 12 %;\
  /ELSE \
    /test pm_ := 0 %;\
  /ENDIF %;\
  /test st_ := strcat(hours_, ':', min_) %;\
  /IF (pm_) \
    /test st_ := strcat(st_, 'pm') %;\
  /ELSE \
    /test st_ := strcat(st_, 'am') %;\
  /ENDIF %;\
  /return st_

;-----------------------------------------------------------------------------
; num2commas()
;
; returns a number formatted with comma's
; USAGE:
; x := num2commas(value)
;
; x     : Value returned with commas. Example 1,345 from 1345.
; value : Any number.
;
; NOTES: x can also be floating point.
;-----------------------------------------------------------------------------
/def num2commas = \
  /let st_ %{1} %;\
  /let pos_ 0 %;\
  /let count_ 0 %;\
  /let count2_ 0 %;\
  /let st2_ 0 %;\
  /let frac_ 0 %;\
  /test st2_ := '' %;\
  /test pos_ := strstr(st_, '.') %;\
  /IF (pos_ > -1) \
    /test count_ := pos_ %;\
    /test frac_ := substr(st_, pos_) %;\
  /ELSE \
    /test count_ := strlen(st_) %;\
    /test frac_ := '' %;\
  /ENDIF %;\
  /WHILE (--count_ >= 0) \
    /IF ( (count2_ = 3) & (substr(st_, count_, 1) !~ '-') ) \
      /test st2_ := strcat(',', st2_) %;\
      /test count2_ := 0 %;\
    /ENDIF %;\
    /test st2_ := strcat(substr(st_, count_, 1), st2_) %;\
    /test ++count2_ %;\
  /DONE %;\
  /return strcat(st2_, frac_)

;-----------------------------------------------------------------------------
; send_macro()
;
; Sends a macro to tinyfugue
; USAGE:
; send_macro("macro")
;
; "macro"   : A tinyfugue macro like "/help"
;
; NOTES: If "macro" doesn't begin with a "/" it will send it to the world
;        you're currently connected to.
;-----------------------------------------------------------------------------
/def send_macro = \
  /def send_macro2 = \
    %* %;\
  /send_macro2 %;\
  /undef send_macro2

;-----------------------------------------------------------------------------
; strip_left()
;
; Strips all leading spaces from a string.
; USAGE:
; x := strip_left("string")
;
; x        : Returned "string" with no leading spaces.
; "string" : Any string
;-----------------------------------------------------------------------------
/def strip_left = \
  /let st_ 0 %;\
  /test st_ := {1} %;\
  /let count_ -1 %;\
  /let len_ $[strlen(st_)] %;\
  /WHILE ( (++count_ < len_) & (substr(st_, count_, 1) =~ ' ') ) \
  /DONE %;\
  /return substr(st_, count_)

;-----------------------------------------------------------------------------
; strip_right()
;
; Strips all trailing spaces from a string.
; USAGE:
; x := strip_right("string")
;
; x        : Returned "string" with no leading spaces.
; "string" : Any string
;-----------------------------------------------------------------------------
/def strip_right = \
  /let st_=%* %;\
  /RETURN st_

;-----------------------------------------------------------------------------
; parse_string()
;
; This function will parse a string by the separator you choose and place the
; elements back into an array of your choosing.
; USAGE:
; parse_string("array", "sep", "string")
;
; "array"  : get_array("data", 0) = number of elements returned.
;            get_array("data", 1, 2, 3..) = "string" parsed out.
; "sep"    : The divider of elements in string.
; "string" : The string you want parsed.
;
; EXAMPLE:
; parse_string("data", " ", "hp(45/100) sp(450/500)")
;
; get_array("data", 0) = 2
; get_array("data", 1) = "hp(45/100)"
; get_array("data", 2) = "sp(450/500)"
;-----------------------------------------------------------------------------
/def parse_string = \
  /let array_ %{1} %;\
  /let sep_ 0 %;\
  /test sep_ := {2} %;\
  /let data_ 0 %;\
  /test data_ := {3} %;\
  /let count_ -1 %;\
  /let parcount_ 0 %;\
  /let ch_ 0 %;\
  /let st_ 0 %;\
  /let len_ 0 %;\
  /test ch_ := '' %;\
  /test st_ := '' %;\
  /test len_ := strlen(data_) %;\
  /test purge_array(array_) %;\
  /WHILE (++count_ <= len_) \
    /test ch_ := substr(data_, count_, 1) %;\
    /IF ( (ch_ =~ sep_) | (count_ = len_) ) \
      /test ++parcount_ %;\
      /test put_array(array_, parcount_, st_) %;\
      /test st_ := '' %;\
    /ELSE \
      /test st_ := strcat(st_, ch_) %;\
    /ENDIF %;\
  /DONE %;\
  /test put_array(array_, 0, parcount_)

;-----------------------------------------------------------------------------
; search_literal()
;
; Search for a character in a string of characters.  Characters will be
; skipped in the string that start with a literal-switch.  The position of
; the character found will be based on the string returned with no literal
; switches.
; USAGE:
; x := search_literal("data", "string", char, litchar)
;
; x         : Position of char in "string". -1 returned if nothing found.
; "data"    : Record that contains the following:
;             /rec :data.pos:    = position
;             /rec :data.st:     = "string" with no literal switches.
; "string"  : String being searched.
; char      : Char to search for in "string" skipping literal switches.
; litchar   : The literal-switch of your choosing.
;
; EXAMPLES:
; x := search_literal("data", "/A/C/car/rocks", "c", "/")
; RETURNS:
; x := 7
; /rec :data.pos:   = 7
; /rec :data.st:    = "ACcarrocks"
;
; NOTES: To use a literal-switch as data in the "data" use two of them in
;        a row, like "\\" would mean "\"
;-----------------------------------------------------------------------------
/def search_literal = \
  /let rec_ 0 %;\
  /test rec_ := {1} %;\
  /let string_ 0 %;\
  /test string_ := {2} %;\
  /let char_ 0 %;\
  /test char_ := {3} %;\
  /let litchar_ 0 %;\
  /test litchar_ := {4} %;\
  /let string2_ 0 %;\
  /let lastch_ 0 %;\
  /let ch_ 0 %;\
  /let count_ -1 %;\
  /let pos_ -1 %;\
  /let length_ $[strlen(string_)] %;\
  /test string2_ := '' %;\
  /test lastch_ := '' %;\
  /test ch_ := '' %;\
  /WHILE (++count_ <= length_) \
    /test lastch_ := ch_ %;\
    /test ch_ := substr(string_, count_, 1) %;\
    /IF (ch_ =~ litchar_) \
      /IF (substr(string_, count_ + 1, 1) =~ litchar_) \
        /test string2_ := strcat(string2_, litchar_) %;\
        /test ++count_ %;\
      /ENDIF %;\
    /ELSE \
      /IF ( (lastch_ !~ litchar_) & (ch_ =~ char_) & (pos_ = -1) ) \
        /test pos_ := strlen(string2_) %;\
      /ENDIF %;\
      /test string2_ := strcat(string2_, ch_) %;\
    /ENDIF %;\
  /DONE %;\
  /test del_rec(rec_, '') %;\
  /test add_rec(rec_, 'pos', pos_) %;\
  /test add_rec(rec_, 'st', string2_) %;\
  /RETURN pos_

;-----------------------------------------------------------------------------
; find_string()
;
; Finds a string within a string and return the starting position, end
; position and length in a virtual array.
; USAGE:
; x := find_string("pos", "string", "search", stringpos, skip)
;
; x          : -1 no string found, 1 string found.
; "pos"      : Position data returned in a record.
;              /rec :pos.found:        = 1 if string found -1 if not found.
;              /rec :pos.st:           = search data found.
;              /rec :pos.start:        = starting location in "string".
;              /rec :pos.end:          = ending location in "string".
;              /rec :pos.len:          = length of sub string found.
; "string"   : String being searched.
; "search"   : String your searching for.
; stringpos  : Position in string to start searching at.
; skip       : Number of matches to skip before returning a real match.
;
; EXAMPLES:
; find_string("pos", "Status is currently 45.65 degrees", "y * d", 0, 0)
; RETURNS:
; /rec :pos.found:        = 1           {string found}
; /rec :pos.st:           = "y 45.65 d" {actual data found}
; /rec :pos.start:        = 18          {starting position}
; /rec :pos.end:          = 26          {ending position}
; /rec :pos.len:          = 9           {length}
;
; To pull out just the 45.65 in the example above you would need to do:
; find_string("pos", "Status is currently 45.65 degrees", "y (*) d", 0, 0)
;
; NOTES: Anything in () is returned, you can only have one set of () and
;        one "*" in your search.  Use ~( ~) ~* if you want that as part of
;        your search data. When using () with *, the * must be inside the
;        () or you may get strange results.
;-----------------------------------------------------------------------------
/def find_string2 = \
  /let string_ 0 %;\
  /test string_ := {1} %;\
  /let search_ := 0 %;\
  /test search_ := {2} %;\
  /let skip_ %{3} %;\
  /let searchl_ $[strlen(search_)] %;\
  /let count_ 0 %;\
  /let count2_ 0 %;\
  /let done_ 0 %;\
  /let pos_ 0 %;\
  /let pos2_ 0 %;\
  /WHILE (!done_) \
    /test pos_ := strstr(string_, search_) %;\
    /IF (pos_ > -1) \
      /test ++count2_ %;\
      /IF (count2_ > skip_) \
        /test pos_ := pos2_ + pos_ %;\
        /test done_ := 1 %;\
      /ELSE \
        /test string_ := substr(string_, pos_ + searchl_) %;\
        /test pos2_ := pos2_ + pos_ + searchl_ %;\
      /ENDIF %;\
    /ELSE \
      /test done_ := 1 %;\
    /ENDIF %;\
  /DONE %;\
  /IF (count2_ <= skip_) \
    /test pos_ := -1 %;\
  /ENDIF %;\
  /RETURN pos_

/def find_string3 = \
  /let string_ 0 %;\
  /test string_ := {1} %;\
  /let searchL_ 0 %;\
  /test searchL_ := {2} %;\
  /let searchR_ 0 %;\
  /test searchR_ := {3} %;\
  /let skip_ %{4} %;\
  /let count_ 0 %;\
  /let count2_ 0 %;\
  /let done_ 0 %;\
  /let posL_ 0 %;\
  /let posR_ 0 %;\
  /let pos2_ 0 %;\
  /WHILE (!done_) \
    /test posL_ := strstr(string_, searchL_) %;\
    /IF (posL_ > -1) \
      /test string_ := substr(string_, posL_ + strlen(searchL_)) %;\
      /test posR_ := strstr(string_, searchR_) %;\
      /IF (posR_ > -1) \
        /test ++count2_ %;\
        /IF (count2_ > skip_) \
          /test length_ := posR_ + strlen(searchL_) + strlen(searchR_) %;\
          /test pos_ := pos2_ + posL_ %;\
          /test done_ := 1 %;\
        /ELSE \
          /test pos2_ := pos2_ + posL_ + strlen(searchL_) %;\
        /ENDIF %;\
      /ELSE \
        /test done_ := 1 %;\
      /ENDIF %;\
    /ELSE \
      /test done_ := 1 %;\
    /ENDIF %;\
  /DONE %;\
  /IF (count2_ <= skip_) \
    /test pos_ := -1 %;\
  /ENDIF %;\
  /RETURN pos_

/def find_string = \
  /let rec_ 0 %;\
  /test rec_ := {1} %;\
  /let string_ 0 %;\
  /test string_ := {2} %;\
  /let search_ 0 %;\
  /test search_ := {3} %;\
  /let stringpos_ %{4} %;\
  /let skip_ 0 %;\
  /test skip_ := {5} %;\
  /let found_ -1 %;\
  /let pos_ -1 %;\
  /let pos2_ 0 %;\
  /let end_ 0 %;\
  /let length_ 0 %;\
  /let left_ 0 %;\
  /let right_ 0 %;\
  /let st_ 0 %;\
  /let searchL_ 0 %;\
  /let searchR_ 0 %;\
  /test right_ := 0 %;\
  /test st_ := '' %;\
  /test searchL_ := '' %;\
  /test searchR_ := '' %;\
  /test string_ := substr(string_, stringpos_) %;\
  /IF (search_ !~ '') \
    /IF ( (search_ =~ '*') | (search_ =~ "(*)") ) \
      /IF ( (skip_ = 0) & (string_ !~ '') )\
        /test found_ := 1 %;\
        /test pos_ := 0 %;\
        /test end_ := strlen(string_) - 1 %;\
        /test length_ := end_ + 1 %;\
        /test st_ := string_ %;\
      /ENDIF %;\
    /ELSE \
      /test del_rec(rec_, '') %;\
      /test left_ := search_literal(rec_, search_, '(', '~') %;\
      /test right_ := search_literal(rec_, search_, ')', '~') %;\
      /test pos2_ := search_literal(rec_, search_, '*', '~') %;\
      /test search_ := get_rec(rec_, 'st') %;\
      /IF (left_ + right_ > 0) \
        /test search_ := strcat(substr(search_, 0, left_), \
                                  substr(search_, left_ + 1, right_ - left_ - 1), \
                                  substr(search_, right_ + 1)) %;\
        /test right_ := right_ - 2 %;\
        /IF (pos2_ > -1) \
          /test --pos2_ %;\
        /ENDIF %;\
      /ENDIF %;\
      /IF (pos2_ = -1) \
        /test found_ := find_string2(string_, search_, skip_) %;\
        /IF (found_ > -1) \
          /test pos_ := found_ %;\
          /test length_ := strlen(search_) %;\
          /test end_ := pos_ + length_ - 1 %;\
          /test st_ := search_ %;\
          /test found_ := 1 %;\
          /IF (left_ + right_ > 0) \
            /test length_ := right_ - left_ + 1 %;\
            /test pos_ := pos_ + left_ %;\
            /test end_ := pos_ + right_ - 1 %;\
            /test st_ := substr(string_, pos_, length_) %;\
          /ENDIF %;\
        /ENDIF %;\
      /ELSE \
        /test searchL_ := substr(search_, 0, pos2_) %;\
        /test searchR_ := substr(search_, pos2_ + 1) %;\
        /IF (find_string3(string_, searchL_, searchR_, skip_) > -1) \
          /test found_ := 1 %;\
          /test end_ := pos_ + length_ - 1 %;\
          /test st_ := substr(string_, pos_, length_) %;\
          /IF (left_ + right_ > 0) \
            /test pos_ := pos_ + left_ %;\
            /test end_ := end_ - (strlen(search_) - right_) + 1 %;\
            /test length_ := end_ - pos_ +1 %;\
            /test st_ := substr(string_, pos_, length_) %;\
          /ENDIF %;\
        /ENDIF %;\
      /ENDIF %;\
    /ENDIF %;\
  /ENDIF %;\
  /test del_rec(rec_, '') %;\
  /IF (found_ = 1) \
    /test pos_ := pos_ + stringpos_ %;\
    /test end_ := end_ + stringpos_ %;\
  /ENDIF %;\
  /test add_rec(rec_, 'found', found_) %;\
  /test add_rec(rec_, 'st', st_) %;\
  /test add_rec(rec_, 'start', pos_) %;\
  /test add_rec(rec_, 'end', end_) %;\
  /test add_rec(rec_, 'len', length_) %;\
  /RETURN found_

;-----------------------------------------------------------------------------
; search_replace()
;
; Searches for a string and replaces each occurrence with a new string.
; USAGE
; x := search_replace("record", "string", "search", "replace", count, skip)
;
; x         : -1 no searches done, >=1 = how many searches were done.
; "record"  : /rec :record.replace:    = How many replaces were done. -1 = none
;             /rec :record.st:         = New string with replaced parts.
; "string"  : String to have parts to be replaced.
; "search"  : Sub string in "string" to be replaced.
; "replace" : New sub string that will replace each "search" string found.
; count     : How many replacements to do. 0 = unlimited.
; skip      : How many matches you want to skip before replacing. 0 = no skip.
;
; EXAMPLES:
; search_replace("search", "Status is currently 45.65 degrees", "y (*) d", "34.2", 0, 0)
; RETURNS:
; /rec :search.replace:      = 1 (1 replacement was done)
; /rec :search.st:           = "Status is currently 32.2 degrees"
;
; NOTES: Anything in () is replaced.  You can only have one set of () and
;        one "*" in your search.  Use ~( ~) ~* if you want that as part of
;        your search data. When using () with *, the * must be inside the
;        () or you may get strange results.
;-----------------------------------------------------------------------------
/def search_replace = \
  /let rec_ %{1} %;\
  /let string_ 0 %;\
  /test string_ := {2} %;\
  /let search_ 0 %;\
  /test search_ := {3} %;\
  /let replace_ 0 %;\
  /test replace_ := {4} %;\
  /let count_ %{5} %;\
  /let skip_ %{6} %;\
  /let count2_ 0 %;\
  /let pos_ 0 %;\
  /let done_ 0 %;\
  /let start_ 0 %;\
  /let length_ 0 %;\
  /let end_ 0 %;\
  /WHILE (!done_) \
    /IF (find_string(rec_, string_, search_, pos_, skip_) != -1) \
      /IF ( (count2_ < count_) | (count_ = 0) ) \
        /test ++count2_ %;\
        /test skip_ := 0 %;\
        /test start_ := get_rec(rec_, "start") %;\
        /test length_ := get_rec(rec_, "len") %;\
        /test end_ := start_ + length_ %;\
        /test string_ := strcat(substr(string_, 0, start_), \
                                replace_, \
                                substr(string_, end_)) %;\
        /test pos_ := start_ + strlen(replace_) %;\
      /ELSE \
        /test done_ := 1 %;\
      /ENDIF %;\
    /ELSE \
      /test done_ := 1 %;\
    /ENDIF %;\
  /DONE %;\
  /test del_rec(rec_, '') %;\
  /IF (count2_ = 0) \
    /test count2_ := -1 %;\
  /ENDIF %;\
  /test add_rec(rec_, 'replace', count2_) %;\
  /test add_rec(rec_, 'st', string_) %;\
  /RETURN count2_
;============================================================================;
