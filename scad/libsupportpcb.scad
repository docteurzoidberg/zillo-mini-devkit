coinLargeur=6;
coinLongueur=10;

coteLongueur=16;
coteLargeur=6;

epaisseurPcb=1.6;

margePcbLongueur=2;
margePcbLargeur=1;

hauteurClip=4;
epaisseurClip=0.8;

//un cote support = un coin. un cote. et un autre coin en mirroir X
module supportUnCotePcb(longueurPcb, largeurPcb, doCotes=true, useClips=true, hauteurSupport=6){
	
	translate([-(coinLargeur-margePcbLongueur),-(coinLargeur-margePcbLargeur),0])
		coin(hauteurSupport=hauteurSupport);
	
	translate([longueurPcb,0,0])
		mirror([1,0,0])
			translate([-(coinLargeur-margePcbLongueur),-(coinLargeur-margePcbLargeur),0])
				coin(hauteurSupport=hauteurSupport);
	if(doCotes){
		translate([(longueurPcb/2)-(coteLongueur/2),-(coteLargeur-margePcbLargeur),0])
			cote(useClips=useClips, hauteurSupport=hauteurSupport);
	}
}

//support = deux cotes supports en mirroir Y
module supportPcb(longueurPcb, largeurPcb, hauteurSupport=6, doCotes=true, useClips=true){
	
	supportUnCotePcb(longueurPcb=longueurPcb,largeurPcb=largeurPcb,doCotes=doCotes,useClips=useClips,hauteurSupport=hauteurSupport);
	
	translate([0,largeurPcb,0])
		mirror([0,1,0])
			supportUnCotePcb(longueurPcb=longueurPcb,largeurPcb=largeurPcb,doCotes=doCotes,useClips=useClips,hauteurSupport=hauteurSupport);
}

module coin(){
	difference(){
		cube([coinLongueur, coinLongueur, hauteurSupport]);
		translate([coinLargeur,coinLargeur,-.1])
			cube([coinLongueur, coinLongueur, hauteurSupport+.2]);
		translate([coinLargeur-margePcbLongueur,coinLargeur-margePcbLargeur,hauteurSupport-epaisseurPcb])
			cube([coinLongueur, coinLongueur, hauteurSupport+.1]);
	}
	
}

module clip(epaisseurPcb=1.6,margePcb=1){
	polygon([
		[0,0],
		[epaisseurClip,0],
		[epaisseurClip,epaisseurPcb+.1],
		[epaisseurClip+margePcb,epaisseurPcb+.2],
		[epaisseurClip,epaisseurPcb+.1+hauteurClip],
		[0,epaisseurPcb+.1+hauteurClip],
	]);
}

module cote(useClips=true, hauteurSupport=6){
	
	if(useClips) {
		
		translate([0,coteLargeur-epaisseurClip-margePcbLargeur,hauteurSupport-epaisseurPcb])
			rotate(90,[1,0,0]) 
				rotate(90,[0,1,0])
					linear_extrude(height=coteLongueur)
						 clip();

		cube([coteLongueur, coteLargeur, hauteurSupport-epaisseurPcb]);
		
	}
	else {
		difference(){
			cube([coteLongueur, coteLargeur, hauteurSupport]);
			translate([-.1,coteLargeur-margePcbLargeur, hauteurSupport-epaisseurPcb])
				cube([coteLongueur+.2, coteLargeur, epaisseurPcb+.1]);
		}
	}
	
}

module test_7x5(){
	translate([-4,-5,-3])
		cube([78,60,3]);
	supportPcb(longueurPcb=70,largeurPcb=50, doCotes=true, useClips=true);
}

module test_8x2(){
	difference(){
		translate([-4,-5,0])
			cube([88,30,1.4]);
		translate([2,1,-0.2])
			cube([76,18,1.8]);
	}
	supportPcb(longueurPcb=80,largeurPcb=20, doCotes=true, useClips=true, hauteurSupport=3);
}

//test_7x5();
test_8x2();