#To (re)compile C bytecode:
#
#/path/to/mruby/bin/mrbc -Bliquid_crystal_example -oliquid_crystal_example.c liquid_crystal_example.rb
#

class LiquidCrystalExample
	include Arduino

	def initialize
    @lcd = LiquidCrystal.new(8, 9, 4, 5, 6, 7) # this depends on your specific liquid crystal shield
    # set up the LCD's number of columns and rows:
    @lcd.begin(16, 2)
    # Print a message to the LCD.
    @lcd.print("hello, mruby!");
	end

	def run
    @lcd.setCursor(0,1)
    @lcd.print((millis / 1000).to_i.to_s)
	end
end


