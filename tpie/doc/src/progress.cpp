// Include the file that will allow us to use the indicator.
#include <progress_indicator_arrow.h>

int main(int argc, char *argv[]) { 
    
    int upper = 32*1024;

    //  Count from 0 to upper in units of 5 steps.
    progress_indicator_arrow* indicator = new 
	progress_indicator_arrow("Title of an indicator", "Tests so far:",
				    0, upper, 5);

    for (int i=0; i< upper; i++) {
	//  Advance on every fifth step.
	if (!(i % 5)) {
	    indicator->step();
	}
    }
    indicator->done("Done.");

    upper = upper * 1024;
    indicator->set_title("Checking the percentage-based indicator");

    indicator->set_description("Pass 1/3:");
    //  Update the display with every 1.0% of progress
    indicator->set_percentage_range(0,upper);
    indicator->init();
    for (int i=0; i< upper; i++) {
	indicator->step_percentage();
    }

    indicator->set_description("Pass 2/3:");
    //  Update the display with every 0.1% of progress
    indicator->set_percentage_range(0,upper,1000);
    indicator->init();
    for (int i=0; i< upper; i++) {
	indicator->step_percentage();
    }

    indicator->set_description("Pass 3/3:");
    //  Update the display with every 5.0% of progress
    indicator->set_percentage_range(0,upper,20);
    indicator->init();
    for (int i=0; i< upper; i++) {
	indicator->step_percentage();
    }

    indicator->done("Done as well.");
    
    delete indicator;

    return 0;
}
