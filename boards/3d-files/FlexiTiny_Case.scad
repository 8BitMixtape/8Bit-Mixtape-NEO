



//14 9
WallHeight = 12;
BaseHeight = 1.6;

 Body();
 Cover();


 module Cover(){
difference(){
translate([50,0,1+BaseHeight-WallHeight/2]) caseCover();
    translate([50,0,0]) linear_extrude(height = BaseHeight*8, center = true, convexity = 10, twist = 0) import("TinyCase_TopCut2.dxf", convexity=3);
    translate([50,0,-1.6]) linear_extrude(height = BaseHeight, center = true, convexity = 10, twist = 0) import("TinyCase_TopCut.dxf", convexity=3);
  translate([64,42,1.6]) rotate([90, 0, 0])  cube([8, 8, ,30], center=true);    

  }
}
 module caseCover(){
  linear_extrude(height = 2.6, center = false, convexity = 10, twist = 0) import("TinyCase_cover.dxf", convexity=3);
  
  translate([0,0,0]) linear_extrude(height = BaseHeight-0.2, center = false, convexity = 10, twist = 0) import("TinyCase_base.dxf", convexity=3);
    
 translate([6.2,28,-3.6+WallHeight/2]) cylinder(h=1.6, r=2, center=true, $fn=30);
 translate([21.8,28,-3.6+WallHeight/2]) cylinder(h=1.6, r=2, center=true, $fn=30);
     
 }

 module Body(){
difference(){
    caseBody();
    //USB-C plug
    translate([14,3,1.2+BaseHeight-WallHeight/2]) cube([9,12,4],true);
    //audio-Jack
    translate([14,36,-3.6+WallHeight/2]) rotate([90, 0, 0]) cylinder(h=20, r=3, center=true, $fn=30);
    //side-port
    translate([26,16,-3.6+WallHeight/2]) cube([10,8,4],true);
}   
}
    
   
 module caseBody(){
  linear_extrude(height = WallHeight, center = true, convexity = 10, twist = 0) import("TinyCase_wall.dxf", convexity=3);
  
  translate([0,0,BaseHeight/2-WallHeight/2]) linear_extrude(height = BaseHeight, center = true, convexity = 10, twist = 0) import("TinyCase_base.dxf", convexity=3);
     
  translate([14,28,0.4+BaseHeight-WallHeight/2]) cube([5,4,3.2],true);
  translate([14,30,0.8+BaseHeight-WallHeight/2]) cube([10,3,4.8],true);
     
  translate([3,28,0.8+BaseHeight-WallHeight/2]) cube([2,10,4.8],true);
  translate([3,14,0.8+BaseHeight-WallHeight/2]) cube([2,10,4.8],true);
  translate([25,28,0.8+BaseHeight-WallHeight/2]) cube([2,10,4.8],true);
  translate([25,14,0.8+BaseHeight-WallHeight/2]) cube([2,10,4.8],true);
 }
 

 
 


