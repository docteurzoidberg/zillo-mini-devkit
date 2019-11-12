
tilesX=8;
tilesY=8;
tileRadius=2;

tileSpacing=tileRadius/3;
outRadius= tileRadius+tileSpacing /2;
inRadius = outRadius * (sqrt(3)/2);

module tile(h) {
	$fn=6; //resolution des cercles
	cylinder(baseHeight+h, outRadius, outRadius);
	translate([0,0,baseHeight+h])
		color([1-h,0,0]) cylinder(.6, outRadius, inRadius);
}

module map(w,h) {
	
	minTilesY=1;
	maxTilesY=12;
	lastTilesY=0;
	
	for(x=[0:tilesX-1]) {

		curTilesY = rands(minTilesY, maxTilesY,1)[0];
		
		for(y=[0:tilesY-1]) {
	   
			randHeight = rands(0,100,1)[0];
			randHole = rands(0,1,1)[0];
			  
			if(randHole>0){
				offsetPos = (x%2) != 0 ? inRadius : 0 ;
				translate([
					x * outRadius * 1.5,
					y * inRadius * 2 + offsetPos,
					0])
				  //tile( x*2+y*2 + (randHeight/20));
					tile( (x*3)+(y*3));
			}
		}
	}
}

baseHeight=6;
baseDiameter=((tilesX*1.4)*(tileRadius*2));

module stamp() {
	$fn=100;
	d=(tilesX*1.4)*(tileRadius*2);
	ofX=((tilesX-1)*(outRadius*2))/2;
	ofY=((tilesY-1)*(inRadius*2.5))/2;;
	//translate([outRadius,inRadius,0])
	union() {
		//translate([0,0,-2])
			//cylinder(baseHeight-0.8, d/2,d/2);
		translate([-ofX,-ofY,0])
			translate([outRadius,inRadius,0])
				map(tilesX,tilesY);
	}
	echo(d);
}

scale(1.5)
	stamp();


