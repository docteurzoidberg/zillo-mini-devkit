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

module colonne1(x=0,y=0,height=-1,shell=1.6,topstyle="flat"){
	h=(x+1)*4+(y+1)*4;
	difference() {
		cube([led_l+(shell*2),led_w+(shell*2),h]);
		translate([shell,shell,0])
			cube([led_l,led_w,h +.1]);
	}
	//toit plat
	translate([0,0,h])
		cube([led_l+(shell*2),led_w+(shell*2),2]);
}


module colonne2(x=0,y=0,height=-1,shell=0.6,topstyle="flat"){
	h=(x+1)*5+(y+1)*5;
	
	rc = (sqrt(3)/2)*5;
	ofX=((rc+2*shell)/2)+1/3;
	ofY=((rc+2*shell)/2)+1/3;
	translate([ofX,ofY])
	rotate([0,0,30])
	difference() {
		cylinder(h,rc+shell,rc+shell, $fn=6);
		translate([0,0,-.1])
			cylinder(h +.2,rc,rc, $fn=6);
	}
}


module diffuser(type="colonne1") {
	
	//base
	difference(){
		cube([diffuser_l,diffuser_w,diffuser_shell]);
		translate([diffuser_walls+1-.2,diffuser_walls+1-.2,-.1])
			cube([diffuser_l-diffuser_walls*2-2+.4,diffuser_w-diffuser_walls*2-2+.4,diffuser_shell+.2]);
	}
	
	//trous base
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
					if(type=="colonne1")
						colonne1(x=x2,y=y2, shell=colonnes_walls);
					if(type=="colonne2")
						colonne2(x=x2,y=y2, shell=colonnes_walls);
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

diamic=9.4;
lmic=20;
wmic=14;
hmic=5;

module support_micro() {
	ofXmic=3+diamic/2;
	ofYmic=wmic/2;
	oftrou=1.2;
	diatrou=1.8;
	wpins=3;
	lpins=9;
	
	difference() {
		cube([lmic,wmic,hmic]);
		translate([ofXmic,ofYmic,-.1])
			cylinder(hmic+.2,diamic/2,diamic/2, $fn=100);
		translate([oftrou,oftrou,-.1])
			cylinder(hmic+.2,diatrou/2,diatrou/2, $fn=100);
		translate([oftrou,wmic-oftrou,-.1])
			cylinder(hmic+.2,diatrou/2,diatrou/2, $fn=100);
		translate([lmic-wpins,(wmic-lpins)/2,hmic-1.6])
			cube([wpins,lpins,10]);
	}
	
}
//support_micro();

diaspk=20;
hspk=1;
hsupportspk=9;
diasupportspk=30;

module support_speaker() {
	difference() {
		cylinder(hsupportspk, (diasupportspk/2),(diasupportspk/2), $fn=50);
		for(i=[0:10]){
			translate([i,0,hsupportspk-2-hspk])
				cylinder(hspk+.2,diaspk/2,diaspk/2, $fn=50);
		}
		translate([0,0,-.1])
			cylinder(hsupportspk-2-hspk+.2,(diaspk-2)/2,(diaspk-2)/2, $fn=50);
	}
	
}

//support_speaker();

module preview(){
	top();
	translate([-2,-2,socle_h-diffuser_shell])
		%diffuser();
	translate([-2,-2,-diffuser_shell])
		bottom();
}


preview();
//diffuser(type="colonne1");
//diffuser(type="colonne2");

//top();
//bottom();


