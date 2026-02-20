#include <systemc>
using namespace sc_core;

// DUT:vending machine FSM

SC_MODULE(vendingmachine){
  //inputs
  sc_in<bool> clk;
  sc_in<bool> reset;
  sc_in<bool> coin50;
  sc_in<bool> coin100;
  
  //ouputs
  sc_out<bool> dispense;
  sc_out<bool> change50;
  
  // expose state for functional coverage
  sc_out<int> state_out;
  
  //state encoding 
  enum state_t { idle,credit_50,change_50,dispenser };
  sc_signal<state_t> state,next_state;
  
  //combinationl:next-state + outputs
  
  void comb_logic(){
    //default outputs
    dispense.write(false); // set 0 as default
    change50.write(false);
    
    //default next state stays the same
    next_state.write(state.read());
    
    switch (state.read()) {
    case idle:
      if (coin50.read()) {
        next_state.write(credit_50);
      }else if (coin100.read()){
        next_state.write(dispenser);
      } else{
        	next_state.write(idle);
      }
      break;
      
    case credit_50:
      if (coin50.read()) {
        next_state.write(dispenser);
      }else if(coin100.read()){
        next_state.write(change_50);
      }else{
        next_state.write(credit_50);
      }
      break;
      
    case change_50:
    	change50.write(true);
        //after giving change will dispense cola next
        next_state.write(dispenser);
        break;
    
    case dispenser:
    	dispense.write(true);
      	next_state.write(idle);
        break;
      
    default:
      	next_state.write(idle);
      	break;
    
    }
  }
  
  
  void seq_logic(){
    //start in idle
    state.write(idle);
    
    while(true){
      wait(clk.posedge_event());//wait for next rising edge of clock
    if(reset.read()){
      state.write(idle);
    }else{
      state.write(next_state.read());
    }
   }   
  }  
    // publish the current state for testbench coverage
  void export_state() {
    state_out.write(static_cast<int>(state.read()));
  }  
  
  SC_CTOR(vendingmachine){
  	  SC_METHOD(comb_logic);// how to run 
      sensitive << state << coin50 << coin100;
      
      SC_THREAD(seq_logic);
  
      
    SC_METHOD(export_state);
    sensitive << state;
    dont_initialize();
  }  
  
};
