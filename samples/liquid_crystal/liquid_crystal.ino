#if defined(ARDUINO) && (ARDUINO==153)
#include <Wire.h>
#endif

#include <LiquidCrystal.h>

#ifdef MPIDE
#else
#include <Arduino.h>
#endif

#include <stdio.h>

#include "mruby.h"
#include "mruby/class.h"
#include "mruby/value.h"
#include "mruby/string.h"
#include "mruby/irep.h"

//for chipKIT Max32
#ifdef MPIDE
//enhance heap
#define CHANGE_HEAP_SIZE(size) __asm__ volatile ("\t.globl _min_heap_size\n\t.equ _min_heap_size, " #size "\n")
CHANGE_HEAP_SIZE(110 * 1024);

#endif

// This is a hacked version of the blinker example for the liquid crystal shield

mrb_state *mrb;
mrb_value liquid_crystal_obj;
int ai;
size_t total_allocated;

extern const uint8_t liquid_crystal_example[];

void print_exception(mrb_state *mrb){
	mrb_value str;
	str = mrb_funcall(mrb, mrb_obj_value(mrb->exc), "inspect", 0);

	//We can't use Serial.println() because str may not ends with NULL character.
	Serial.write((uint8_t *)RSTRING_PTR(str), RSTRING_LEN(str));
	Serial.println();
}

// custom allocator to check heap shortage.
void *myallocf(mrb_state *mrb, void *p, size_t size, void *ud){
	if (size == 0){
		free(p);
		return NULL;
	}

	void *ret = realloc(p, size);
	if (!ret){
		/*
			Reaches here means mruby failed to allocate memory.
			Sometimes it is not critical because mruby core will retry allocation
			after GC. 
		*/

		Serial.print("memory allocation error. requested size:");
		Serial.println(size, DEC);

		Serial.flush();

		//Ensure serial output received by host before proceeding.
		delay(200);
		return NULL;
	}
	total_allocated += size;
	return ret;
}

void setup(){

	Serial.begin(9600);
	Serial.println("Hello Arduino");

	total_allocated = 0;

	//Init mruby
	Serial.println("mrb_open()...");
	delay(100);
	mrb = mrb_open_allocf(myallocf, NULL);
	if (mrb){
		Serial.println("mrb_open()...done.");
		Serial.print("total allocated : ");
		Serial.println(total_allocated,DEC);
	}else{
		Serial.println("mrb_open()...failed.");
		Serial.print("total allocated : ");
		Serial.println(total_allocated,DEC);
		return;
	}

	char* class_name = "LiquidCrystalExample";

	//Load bytecode
	Serial.print("loading ");
	Serial.print(class_name);
	Serial.println(" bytecode...");
	delay(100);
	mrb_load_irep(mrb, liquid_crystal_example);
	if (mrb->exc){
		Serial.print(class_name);
		Serial.println(" bytecode loading...failed.");
		print_exception(mrb);
		return;
	}else{
		Serial.print(class_name);
		Serial.println("bytecode loading...done.");
	}

	Serial.println("mruby initialized successfully");

	//Get LiquidCrystal class and create instance.
	RClass *arduino_class = mrb_class_get(mrb, class_name);
	if (mrb->exc){
		Serial.print("failed to load class ");
		Serial.print(class_name);
		print_exception(mrb);
		return;
	}

	delay(100);
	liquid_crystal_obj = mrb_class_new_instance(mrb, 0, NULL, arduino_class);

	// has exception occurred?
	if (mrb->exc){
		Serial.print(class_name);
		Serial.println(" instance creation failed");
		print_exception(mrb);
		return;
	}

  Serial.print(class_name);
	Serial.println(" object created.");

	ai = mrb_gc_arena_save(mrb);

}

void loop(){

	mrb_funcall(mrb, liquid_crystal_obj, "run", 0);

	//is exception occure?
	if (mrb->exc){
		Serial.println("failed to run!");
		print_exception(mrb);
		mrb->exc = 0;
		delay(1000);
	}

	mrb_gc_arena_restore(mrb,ai);
}



//required to link with mruby-arduino
void __dummy(){
#if defined(ARDUINO) && (ARDUINO==153)
#else
  random(1,1);
#endif

#ifdef MPIDE
  tone(1,2,3);
#endif

#if defined(ARDUINO) && (ARDUINO==153)
#else
  pulseIn(1,2,3);
  shiftOut(1,2,3,4);
#endif
}
