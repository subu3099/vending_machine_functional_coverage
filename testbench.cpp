//vending machine testbench
#include <systemc>
using namespace sc_core;

#include "design.cpp"

SC_MODULE(tb_vending) {
    sc_in<bool> clk;
    sc_out<bool> reset;
    sc_out<bool> coin50;
    sc_out<bool> coin100;
    sc_in<bool> dispense;
    sc_in<bool> change50;
  
  	//read state from functional coverage
  	sc_in<int> state_in;
  
  	//coverage counter
  	int cov_idle_to_disp = 0;
  	int cov_credit_to_disp = 0;
  	int cov_change_to_disp = 0;
  	//track previous state
  	int prev_state = -1;
  
  	void pulse(sc_out<bool>& sig) {
    	sig.write(true);
      	wait(clk.posedge_event());
      	sig.write(false);
    }
  
  
    void coverage_monitor() {
      	int curr_state = state_in.read();
      	
      	if(prev_state == -1){
          prev_state = curr_state;
          return;
        }
        // dispener =3 (based on enum order in DUT)
        if(curr_state == 3) {
        	//idle=0,credit_50=1,change_50=2
            if(prev_state == 0) cov_idle_to_disp++;
          	else if(prev_state == 1) cov_credit_to_disp++;
            else if(prev_state == 2) cov_change_to_disp++;
        }      
  		
      	prev_state = curr_state;
    }
  
  	void report_coverage(){
    	int hit = 0;
      	int total = 3;
      	
      	std::cout << "\n=== functional coverage report ===\n";
      	
      	std::cout << "cov_1 entered dispenser from idle: "
          		   << (cov_idle_to_disp > 0 ? "hit" : "miss")
          		   << " (count=" << cov_idle_to_disp << ")\n";
      	hit += (cov_idle_to_disp > 0);
      	
      	std::cout << "cov_2 entered dispenser from credit_50: "
          		   << (cov_credit_to_disp > 0 ? "hit" : "miss")
          		   << " (count =" << cov_credit_to_disp << ")\n";
      	hit += (cov_change_to_disp > 0);
      
        std::cout << "cov_2 entered dispenser from change_50: "
          		   << (cov_change_to_disp > 0 ? "hit" : "miss")
          		   << " (count =" << cov_change_to_disp << ")\n";
      	hit += (cov_change_to_disp > 0);
      
      	double percent = 100.0 * hit /total;
      	std::cout << "total: "<< hit << "/" << total
          		   << "=> " << percent << "%\n";
      	std::cout << "=================================\n\n";
    }

    void stimulus() {
        reset.write(true);
        coin50.write(false);
        coin100.write(false);

        wait(clk.posedge_event());
        wait(clk.posedge_event());

        reset.write(false);
        wait(clk.posedge_event());
        //test 1: 50 + 50 
        coin50.write(true);
        wait(clk.posedge_event());
        coin50.write(false);

        wait(clk.posedge_event());

        coin50.write(true);
        wait(clk.posedge_event());
        coin50.write(false);

        wait(clk.posedge_event());
      	wait(clk.posedge_event());
        // test 2 : 100
        coin100.write(true);
        wait(clk.posedge_event());
        coin100.write(false);
      
      	wait(clk.posedge_event());
      	wait(clk.posedge_event());
        // test 3 : 50 + 100 (overpay)
      	coin50.write(true);
        wait(clk.posedge_event());
        coin50.write(false);
      
      	wait(clk.posedge_event());
      
      	coin100.write(true);
        wait(clk.posedge_event());
        coin100.write(false);
      	
        wait(clk.posedge_event());
      	wait(clk.posedge_event());
      	wait(clk.posedge_event());
      	wait(clk.posedge_event());
      
      	
        // print functional coverage report
      	report_coverage();
        sc_stop();
    }
  	// it will run every clock cycle
  	

    SC_CTOR(tb_vending) {
        SC_THREAD(stimulus);
        sensitive << clk.pos();
        dont_initialize();
      
        SC_METHOD(coverage_monitor);
      	sensitive << state_in;
      	dont_initialize();
    
    }
};

//sc_main is toplevel(we have dut and tb)
int sc_main(int argc, char* argv[]) {
    sc_clock clk("clk", 10, SC_NS);

    sc_signal<bool> reset;
    sc_signal<bool> coin50;
    sc_signal<bool> coin100;
    sc_signal<bool> dispense;
    sc_signal<bool> change50;
   
    sc_signal<int> state_sig;

    vendingmachine dut("dut");
    tb_vending tb("tb");
  
    dut.clk(clk);
    dut.reset(reset);
    dut.coin50(coin50);
    dut.coin100(coin100);
    dut.dispense(dispense);
    dut.change50(change50);
  
    dut.state_out(state_sig);

    
    tb.clk(clk);
    tb.reset(reset);
    tb.coin50(coin50);
    tb.coin100(coin100);
    tb.dispense(dispense);
    tb.change50(change50);
  
    tb.state_in(state_sig);
  
  	
    sc_trace_file* tf = sc_create_vcd_trace_file("vending_machine");

	// Trace clock and signals
	sc_trace(tf, clk, "clk");
	sc_trace(tf, reset, "reset");
	sc_trace(tf, coin50, "coin50");
	sc_trace(tf, coin100, "coin100");
	sc_trace(tf, dispense, "dispense");
	sc_trace(tf, change50, "change50");
  	sc_trace(tf,state_sig,"state");
  
    sc_start();
    sc_close_vcd_trace_file(tf);

    return 0;
}
    
    
