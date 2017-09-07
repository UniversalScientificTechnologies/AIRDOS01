h = 30;      // Výška horní části
s = 4;      // Výška spodní části
a = 4;       // Šířka krabičky (počet obsazených dírek)
b = 9;      // Délka krabičky (počet obsazených dírek)
d = 1.8;     // Průměr trnů a šířka hlavních příček (>1)
c = 0.8;   // Tloušťka stěn

drat = 1.5;  // Průměr/výška otvoru na dráty

okraj = 2;  // šířka horního okraje

clear = 0.175;      // K otvoru na matičku
nut_size = 6.6;     // K otvoru na matičku

MLAB_grid = 10.16;
pedestal_height = 2; 

ot = 6*MLAB_grid/11;         //Vzdálenost okraje od děr v plošňáku pro stranu s tunelem
o = MLAB_grid/2+d/2;   //Vzdálenost zbylých okrajů od děr v plošňáku

$fn=20;

roh =0.5; // Zaoblení hran

tld = 2.8; //tloušťka destičky
rantv = tld - 0.8; //výška rantlíku a středu
draz = 0.2; //zvětšení drážky pro krabičku (kvůli roztažení plastu při tisku)
rants = 1.2; //šířka rantlíku
pomoc_kraj = rants+(2*draz+okraj)/2;

// Kvádr a válec se zaoblenými hranami
module roundcube(size,center=true,corner) {
  minkowski() {
    cube(size,center);
    sphere(corner);
  }
}

module roundcylinder(size, r, center=true, corner) {
  minkowski() {
    cylinder(size, r, r, center);
    sphere(corner);
  }
}



// Vytvoření desky

module deska() {
difference(){
   translate([-pomoc_kraj,-pomoc_kraj,0])
     cube([(MLAB_grid*(b-1)+2*ot+2*pomoc_kraj)/2, (MLAB_grid*(a-1)+2*o+2*pomoc_kraj)/2, tld]);

     translate([0,(MLAB_grid*(a-1))/2+o,tld]) 
      cube([okraj+2*draz, MLAB_grid*(a-1)+2*o, 2*rantv],center = true);
     
     translate([(MLAB_grid*(b-1))/2+ot,0,tld])       
      cube([MLAB_grid*(b-1)+2*ot, okraj+2*draz, 2*rantv],center = true); 
    
    //Roh žlábku
     translate([0,0,tld]) 
      cylinder(2*rantv, (okraj+2*draz)/2, (okraj+2*draz)/2, center=true);
    
    //Otvor na pacičku
    translate([ot-MLAB_grid,o-3/7*MLAB_grid-draz,tld-rantv])
       cube([MLAB_grid-ot,2*3/7*MLAB_grid+2*draz,2*rantv]);
    

   
} 
  //Pacička

translate([ot-MLAB_grid, o,0]) {
    difference() {  
        hull() {
        translate([0,-3/7*MLAB_grid,0])
            cube([MLAB_grid-ot,2*3/7*MLAB_grid,tld-rantv]);
            cylinder(tld-rantv, 3/7*MLAB_grid, 3/7*MLAB_grid);
               }
    translate([0,0,1.5])   
    cylinder(3.2, 3.5/2, 3.5/2, center = true);
 
    } 
} 
}


difference(){
union() {
//Zrcadlení
deska();
translate([MLAB_grid*(b-1)+2*ot,0,0]) 
mirror(1,0,1) deska();
translate([0,MLAB_grid*(a-1)+2*o,0])
mirror([0,1,0]) deska();
translate([MLAB_grid*(b-1)+2*ot,0,0]) 
mirror(1,0,1)
translate([0,MLAB_grid*(a-1)+2*o,0])
mirror([0,1,0]) deska();
    
}
     // Tunel
   translate([MLAB_grid/2-d/2, (MLAB_grid*(a-1)+2*o)/2, tld])
    difference() { 
   cube([MLAB_grid+2*draz,1.5*MLAB_grid+2*draz,2*rantv], center = true);
   //cube([MLAB_grid-2-2*draz,1.5*MLAB_grid-2-2*draz,tld], center = true);
    }
    
    //Otvor na dráty
        translate([-5, ((MLAB_grid*(a-1))+2*o)/2,tld])
    cube([13,MLAB_grid-d,2*rantv    ],center = true);
  
  // Konkrétní otvory procházející plošným spojem
 translate([ot+MLAB_grid*1, o, tld/2])
       cylinder(3, 3.5/2, 3.5/2, center = true);
 translate([ot+MLAB_grid*4, o+MLAB_grid*3, tld/2])
       cylinder(3, 3.5/2, 3.5/2, center = true);
 translate([ot+MLAB_grid*5, o, tld/2])
       cylinder(3, 3.5/2, 3.5/2, center = true);
 translate([ot+MLAB_grid*8, o+MLAB_grid*3, tld/2])
       cylinder(3, 3.5/2, 3.5/2, center = true);
 translate([ot+MLAB_grid*1, o+MLAB_grid*3, tld/2])
       cylinder(3, 3.5/2, 3.5/2, center = true);
 translate([ot+MLAB_grid*4, o+MLAB_grid*0, tld/2])
       cylinder(3, 3.5/2, 3.5/2, center = true);
 translate([ot+MLAB_grid*5, o+MLAB_grid*3, tld/2])
       cylinder(3, 3.5/2, 3.5/2, center = true);
 translate([ot+MLAB_grid*8, o+MLAB_grid*0, tld/2])
       cylinder(3, 3.5/2, 3.5/2, center = true);
       
       // zůžení destičky pod plošnými spoji
       translate([ot+MLAB_grid*4.5, o+MLAB_grid*1.5, tld])
       cube([MLAB_grid*(b-1)-3.5, MLAB_grid*(a)-3.5, 2*rantv],center = true);
         
}



/*  ZAKOMENTOVÁNÍ KRABIČKY

translate([(MLAB_grid*(b-1))+2*ot,0,h+c+1]) {
rotate([0,180,0]) {

// Obal klece
    difference() {
        union() {
translate([-d/2,0,0]) 
cube([c, MLAB_grid*(a-1)+2*o, h]);

translate([0,-d/2,0]) 
cube([(MLAB_grid*(b-1))+2*ot, c, h]);

translate([0,0,-d/2]) 
cube([(MLAB_grid*(b-1))+2*ot, (MLAB_grid*(a-1))+2*o, c]);
            
translate([(MLAB_grid*(b-1))+2*ot+d/2-c,0,0]) 
cube([c, MLAB_grid*(a-1)+2*o, h]);

translate([0,(MLAB_grid*(a-1))+2*o+d/2-c,0]) 
cube([(MLAB_grid*(b-1))+2*ot, c, h]);
            
// Horní příčky
translate([okraj/2-d/2, (MLAB_grid*(a-1)+2*o)/2, h-1.5*okraj])
   difference() {
    cube([okraj,MLAB_grid*(a-1)+2*o,3*okraj], center = true);  
    translate([okraj/2,0,0])
    rotate([0,15,0])
    cube([okraj,MLAB_grid*(a-1)+2*o,10*okraj], center = true);   
   } 
        
translate([(MLAB_grid*(b-1))+2*ot-okraj/2+d/2, (MLAB_grid*(a-1)+2*o)/2, h - 1.5*okraj])
    difference() {
    cube([okraj,MLAB_grid*(a-1)+2*o, 3*okraj], center = true);
    translate([-okraj/2,0,0])    
    rotate([0,-15,0])
    cube([okraj,MLAB_grid*(a-1)+2*o,10*okraj], center = true);   
   } 
        
translate([(MLAB_grid*(b-1)+2*ot)/2, okraj/2-d/2, h - 1.5*okraj])
    difference() { 
   cube([MLAB_grid*(b-1)+2*ot, okraj, 3*okraj], center = true);
  
   translate([0,okraj/2,0])
    rotate([-15, 0, 0])
   cube([MLAB_grid*(b-1)+2*ot, okraj, 10*okraj], center = true);
   }
       
 translate([(MLAB_grid*(b-1)+2*ot)/2, (MLAB_grid*(a-1))+2*o-okraj/2+d/2, h - 1.5*okraj])
    difference() { 
   cube([MLAB_grid*(b-1)+2*ot, okraj, 3*okraj], center = true);
   translate([0,-okraj/2,0])
    rotate([15, 0, 0])
   cube([MLAB_grid*(b-1)+2*ot, okraj, 10*okraj], center = true);
   }
   
   }}
 
 }} //Konec rotace