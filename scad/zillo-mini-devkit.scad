leds_x=8;
leds_y=8;
led_spacing=10;

grid_h=2;
led_l=6;
led_w=6;
matrix_length=leds_x*led_spacing;
matrix_width=leds_y*led_spacing;
socle_walls=2;
socle_l=socle_walls+1+matrix_length+1+socle_walls;
socle_w=socle_walls+1+matrix_width+1+socle_walls;
socle_h=22;

diffuser_walls=1;
diffuser_l=socle_l+(diffuser_walls*2)+2;
diffuser_w=socle_w+(diffuser_walls*2)+2;
diffuser_h=16;
diffuser_shell=4;

module grid(){
	firstled_x=socle_walls;
	firstled_y=socle_walls;
	difference(){
		cube([matrix_width,matrix_length,grid_h]);
		for(x=[0:leds_x]){
			for(y=[0:leds_y]){
				posx=firstled_x+(x*led_spacing);
				posy=firstled_y+(y*led_spacing);
				translate([posx,posy,-.1])
					cube([led_l,led_w,6]);
			}
		}
	}
}

module diffuser() {
	//base
	difference(){
		cube([diffuser_l,diffuser_w,diffuser_shell]);
		translate([diffuser_walls+1,diffuser_walls+1,-.1])
			cube([diffuser_l-diffuser_walls*2-2,diffuser_w-diffuser_walls*2-2,diffuser_shell+.2]);
	}
	translate([0,0,diffuser_shell]) {
		difference(){
			cube([diffuser_l,diffuser_w,diffuser_walls]);
			firstled_x=diffuser_walls+socle_walls+4;
			firstled_y=diffuser_walls+socle_walls+4;
			for(x=[0:7]){
				for(y=[0:7]){
					posx=firstled_x+(x*led_spacing);
					posy=firstled_y+(y*led_spacing);
					translate([posx,posy,-.1])
						cube([led_l,led_w,6]);
				}
			}
		}
	}
	
}


module top() {
	difference(){
		cube([socle_l,socle_w,socle_h]);
		translate([socle_walls+1,socle_walls+1,-.1])
			cube([socle_l-socle_walls*2-2,socle_w-socle_walls*2-2,socle_h+.2]);
	}
	translate([socle_walls+1,socle_walls+1,socle_h-grid_h])
		grid();
}


module preview(){
	
top();
translate([-3,-3,socle_h-diffuser_shell+25])
	diffuser();
}


preview();
//diffuser();
//top();
//grid();



