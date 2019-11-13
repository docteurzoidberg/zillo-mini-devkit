leds_x=8;
leds_y=8;
led_spacing=10;
grid_h=2;
led_l=6;
led_w=6;
matrix_length=leds_x*led_spacing;
matrix_width=leds_y*led_spacing;

//validé: correspond parfait a la matrice!
module grid(){
	firstled_x=socle_walls;
	firstled_y=socle_walls;
	difference(){
		cube([matrix_width,matrix_length,grid_h]);
		for(x=[0:leds_x-1]){
			for(y=[0:leds_y-1]){
				posx=firstled_x+(x*led_spacing);
				posy=firstled_y+(y*led_spacing);
				translate([posx,posy,-.1])
					cube([led_l,led_w,6]);
			}
		}
	}
}

//-----------------------------------------------------------------------------------

socle_walls=2;
socle_l=socle_walls+1+matrix_length+1+socle_walls;
socle_w=socle_walls+1+matrix_width+1+socle_walls;
socle_h=22;

module top() {
	difference(){
		cube([socle_l,socle_w,socle_h]);
		translate([socle_walls+1,socle_walls+1,-.1])
			cube([socle_l-socle_walls*2-2,socle_w-socle_walls*2-2,socle_h+.2]);
	}
	translate([socle_walls+1,socle_walls+1,socle_h-grid_h])
		grid();
}

//-----------------------------------------------------------------------------------

diffuser_walls=1;
diffuser_l=socle_l+(diffuser_walls*2)+2;
diffuser_w=socle_w+(diffuser_walls*2)+2;
diffuser_h=16;
diffuser_shell=8;

module diffuser() {
	//base
	difference(){
		cube([diffuser_l,diffuser_w,diffuser_shell]);
		translate([diffuser_walls+1-.2,diffuser_walls+1-.2,-.1])
			cube([diffuser_l-diffuser_walls*2-2+.4,diffuser_w-diffuser_walls*2-2+.4,diffuser_shell+.2]);
	}
	
	translate([0,0,diffuser_shell]) {
		difference(){
			cube([diffuser_l,diffuser_w,diffuser_walls]);
			firstled_x=diffuser_walls+socle_walls+4;
			firstled_y=diffuser_walls+socle_walls+4;
			
			
			for(x=[0:leds_x-1]){
				for(y=[0:leds_y-1]){
					posx=firstled_x+(x*led_spacing);
					posy=firstled_y+(y*led_spacing);
					translate([posx,posy,-.1])
						cube([led_l,led_w,6]);
				}
			}
		}
	}
	
	
	//colonnes 
	
	colonnes_walls=1.2;
	
	translate([0,0,diffuser_shell+diffuser_walls]) {
		
		firstled_x=diffuser_walls+socle_walls+4;
		firstled_y=diffuser_walls+socle_walls+4;
		
		for(x2=[0:leds_x-1]){
			for(y2=[0:leds_y-1]){
				posx2=firstled_x+(x2*led_spacing);
				posy2=firstled_y+(y2*led_spacing);
				translate([posx2-colonnes_walls,posy2-colonnes_walls,0]) { 
					difference() {
						cube([led_l+(colonnes_walls*2),led_w+(colonnes_walls*2),(x2+1)*4+(y2+1)*4]);
						translate([colonnes_walls,colonnes_walls,0])
							cube([led_l,led_w,(x2+1)*4+(y2+1)*4 +.1]);
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------------

bottom_walls=diffuser_walls;
bottom_l=socle_l+(bottom_walls*2)+2;
bottom_w=socle_w+(bottom_walls*2)+2;

module bottom() {
	difference(){
		cube([bottom_l,bottom_w,diffuser_shell]);
		translate([bottom_walls+1-.2,bottom_walls+1-.2,bottom_walls])
			cube([bottom_l-bottom_walls*2-2+.4,bottom_w-bottom_walls*2-2+.4,100]);
	}
}


module preview(){
	top();
	translate([-2,-2,socle_h-diffuser_shell])
		%diffuser();
	translate([-2,-2,-diffuser_shell])
		bottom();
}

//preview();
//diffuser();
//top();
bottom();


