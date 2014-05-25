/* main.cpp
 * Tested with mbed board: FRDM-KL46Z
 * Author: Mark Gottscho
 * mgottscho@ucla.edu
 */

#define NDEBUG

#include "mbed.h"
#include "util.h"
#include "SerialManager.h"
#include <string>
#include "const.h" //for various constants in global scope
#include "INA219.h" //For talking with the power sensors
#include "MAX17043.h" //For talking with the battery fuel gauge

using namespace std;

//The console object is global so that all objects can use its generic serial methods.
SerialManager console(PTE16, PTE17, 115200, false); //For serial console
DigitalOut green(LED1);
DigitalOut red(LED2);

typedef struct {
    string name;
    float voltage;
    float current;
    float power;
} power_state_t;

int main() {
    green = 0; //Indicate power available

    //Turn on red LEDs during init
    red = 0;  
    
    __disable_irq(); //No interrupts during init
    
    //System power state
    power_state_t supply_state;
    power_state_t battery_state;
    power_state_t load_state;
	float battery_soc;
    
    //Init power state
    supply_state.name = "SUPPLY";
    supply_state.voltage = 0;
    supply_state.current = 0;
    supply_state.power = 0;
    
	battery_state.name = "BATTERY";
    battery_state.voltage = 0;
    battery_state.current = 0;
    battery_state.power = 0;
	battery_soc = 0;
   
    load_state.name = "LOAD";
    load_state.voltage = 0;
    load_state.current = 0;
    load_state.power = 0;
    
    //Power/battery monitoring sensors
    INA219 pwr_sens_supply(PTE0, PTE1, PWR_SENS_SUPPLY_I2C_ADDRESS);
    INA219 pwr_sens_battery(PTE0, PTE1, PWR_SENS_BATTERY_I2C_ADDRESS);
    INA219 pwr_sens_load(PTE0, PTE1, PWR_SENS_LOAD_I2C_ADDRESS);
    MAX17043 fuel_gauge(PTE0, PTE1, FUEL_GAUGE_I2C_ADDRESS);
    
    //Init power sensors
    pwr_sens_supply.selfInit();
    pwr_sens_battery.selfInit();
    pwr_sens_load.selfInit();
	fuel_gauge.selfInit();
 
    //Turn off red LED when finished init
    red = 1;

    __enable_irq(); //enable interrupts after initialization

	console.serial.printf("SUPPLY,,,BATTERY,,,,LOAD,,,\r\n");
	console.serial.printf("Voltage (V),Current (A),Power (W),Voltage (V),Current (A),Power (W),SOC(\%),Voltage (V),Current (A),Power (W)\r\n");

    //The Loop
    while(1) {    
        //Sample power state
        supply_state.voltage = pwr_sens_supply.getBusVoltageFloat(true);
        supply_state.power = pwr_sens_supply.getPowerFloat(true);
        supply_state.current = pwr_sens_supply.getCurrentFloat(false);
        
		battery_state.voltage = pwr_sens_battery.getBusVoltageFloat(true);
        battery_state.power = pwr_sens_battery.getPowerFloat(true);
        battery_state.current = pwr_sens_battery.getCurrentFloat(false);
        battery_soc = fuel_gauge.getFloatSOC(true);
		
		load_state.voltage = pwr_sens_load.getBusVoltageFloat(true);
        load_state.power = pwr_sens_load.getPowerFloat(true);
        load_state.current = pwr_sens_load.getCurrentFloat(false);
        
        //Report power state
        console.serial.printf("%f,%f,%f,", supply_state.voltage, supply_state.current * 1000, supply_state.power * 1000);
        console.serial.printf("%f,%f,%f,", battery_state.voltage, battery_state.current * 1000, battery_state.power * 1000);
        console.serial.printf("%f,", battery_soc);
        console.serial.printf("%f,%f,%f\r\n", load_state.voltage, load_state.current * 1000, load_state.power * 1000);
        
        wait(0.5);
    }
}
