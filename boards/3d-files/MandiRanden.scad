// Open Randen Mandi
thicknessWood = 6;


color("SaddleBrown") {

translate ([0,-120,10]) linear_extrude (height = thicknessWood) square ([15,120],center);
translate ([16,-120,10]) linear_extrude (height = thicknessWood) square ([15,120],center);
translate ([32,-120,10]) linear_extrude (height = thicknessWood) square ([15,120],center);
translate ([48,-120,10]) linear_extrude (height = thicknessWood) square ([15,120],center);
translate ([64,-120,10]) linear_extrude (height = thicknessWood) square ([15,120],center);
 translate ([80,-120,10]) linear_extrude (height = thicknessWood) square ([15,120],center);

translate ([96,-120,10]) linear_extrude (height = thicknessWood)  square ([15,120],center);
translate ([112,-120,10]) linear_extrude (height = thicknessWood)  square ([15,120],center);
translate ([128,-65,10]) linear_extrude (height = thicknessWood) square ([15,65],center);
}

// handles and little table
color("DarkOliveGreen") {
translate ([30,-16,90]) linear_extrude (height = thicknessWood) square ([65,15],center);
    
translate ([130,-5,100]) rotate([0,80,0]) cylinder (h=80,d=12);
}

// Logs
color("Grey") {
translate ([14,-105,-30])  rotate([0,0,0]) cylinder(h=43,d=20);
translate ([14,-15,-30])  rotate([0,0,0]) cylinder(h=43,d=20);
translate ([110,-105,-30])  rotate([0,0,0]) cylinder(h=43,d=20);
translate ([130,-15,-30])  rotate([0,0,0]) cylinder(h=43,d=20);
}

// Chimney
color("GhostWhite") {
translate ([0,12,270])  rotate([-90,90,83]) linear_extrude (height = 10) square ([300,72],center);
translate ([-8,-60,270])  rotate([-90,90,0]) linear_extrude (height = 10) square ([300,60],center);
}    

// House Wall
color("BurlyWood") {
translate ([235,0,270])  rotate([10,90,0]) linear_extrude (height = 10) square ([300,235],center);
difference (){
translate ([235,0,270])  rotate([-90,90,0]) linear_extrude (height = 10) square ([300,235],center);
translate ([180,-1,220])  rotate([-90,90,0]) linear_extrude (height = 15) square ([80,60],center);
   }
}

// Logs
color("Maroon") {
translate ([102,0,-10])  rotate([90,90,-9]) cylinder(h=200,d=20);
translate ([165,0,8])  rotate([90,90,-9]) cylinder(h=200,d=20);
}

// Floor
color("DimGrey") {
translate ([135,-200,9])  rotate([0,0,-9]) linear_extrude (height = 10) square ([150,450],center);
}
color("DarkSlateGray") {
translate ([75,-200,-10])  rotate([0,0,-9]) linear_extrude (height = 10) square ([60,220],center);
}
/*
color("DarkSlateGray") {
translate ([-90,-200,-45])  rotate([0,-8,-9]) linear_extrude (height = 10) square ([160,220],center);
}
/*