use <flexbatter.scad>
include <../configuration.scad>


module AIRDOS_prezentace()
{
  
AIRDOS_batter();

translate([10.16-10/2,-17-5.3,vyska_sloupku]) 
AIRDOS_batter_pasek();

translate([5*10.16-10/2,-17-5.3,vyska_sloupku]) 
AIRDOS_batter_pasek();  


translate([0,0,baterka_prumer/2+1.5])   
  
BATERKA();
 
translate([10.16,-17-5.3,vyska_sloupku+3])     
matka();    
 
translate([10.16,-17-5.3+10.16*pocet_poli_mezi_srouby,vyska_sloupku+3])     
matka();    
    
translate([5*10.16,-17-5.3,vyska_sloupku+3])     
matka();    
 
translate([5*10.16,-17-5.3+10.16*pocet_poli_mezi_srouby,vyska_sloupku+3])     
matka();      
    
}  

module AIRDOS_batter_pasek()
{
  
   difference () 
    {
   union()       {   
    cube([10,pocet_poli_mezi_srouby*10.16,2],center=false); 
        translate([10/2,0,0]) 
        cylinder (h = 3, r= 10/2, $fn=20);
        
     translate([10/2,pocet_poli_mezi_srouby*10.16,0]) 
        cylinder (h = 3, r= 10/2, $fn=20);
    }
    
     translate([10/2,0,-0.01]) 
            cylinder (h = vyska_sloupku, r= (prumer_sroubu+0.2)/2, $fn=40);
    
     translate([10/2,pocet_poli_mezi_srouby*10.16,-0.01]) 
            cylinder (h = vyska_sloupku, r= (prumer_sroubu+0.2)/2, $fn=40);
    
    
    }
    
    
       
}

module AIRDOS_batter()
{
        flexbatter();

        translate([10.16,-17-5.3,0])
            SLOUPEK_bocnice(4.5);
        translate([5*10.16,-17-5.3,0])
            SLOUPEK_bocnice(4.5);
            
        translate([10.16,-17-5.3+pocet_poli_mezi_srouby*10.16,0])
            rotate([0, 0, 180])
            SLOUPEK_bocnice(7.5);
        translate([5*10.16,-17-5.3+pocet_poli_mezi_srouby*10.16,0])
            rotate([0, 0, 180])
            SLOUPEK_bocnice(7.5);
}

module SLOUPEK()
{    
translate([0,0,0]) 
    difference () 
    {
        
        cylinder (h = vyska_sloupku, r= sirka_matky/2+sila_materialu/2, $fn=20);
        
        translate([0,0,vyska_sloupku-vyska_matky-0.3]) 
            cylinder (h = vyska_matky+0.31, r= (sirka_matky+0.2)/2, $fn=6);
        
        translate([0,0,-0.01]) 
            cylinder (h = vyska_sloupku, r= (prumer_sroubu+0.2)/2, $fn=40);
               
       
    
    }       
}

module SLOUPEK_bocnice(delka_bocnice)
{    

    
    difference () 
    {
        
     union()       {   
       
         //krychle
         translate([-(sirka_matky/2+sila_materialu/2),0,0]) 
         cube([2*(sirka_matky/2+sila_materialu/2),delka_bocnice,vyska_sloupku-5.5],center=false); 
     
          cylinder (h = vyska_sloupku, r= sirka_matky/2+sila_materialu/2, $fn=20);
         
         }    
        
         
       
        
        translate([0,0,vyska_sloupku-vyska_matky-0.3]) 
            cylinder (h = vyska_matky+0.31, r= (sirka_matky+0.2)/2, $fn=6);
        
        translate([0,0,-0.01]) 
            cylinder (h = vyska_sloupku, r= (prumer_sroubu+0.2)/2, $fn=40);
               
       
    
    }       
}


baterka_prumer=33;
baterka_delka=60;
//https://www.battery.cz/saft-ls-33600-lithiovy-clanek-3-6v-17000mah.html
module BATERKA()
{    
translate([0,0,0]) 
   
   rotate([90, 0, 90])  
        cylinder (h = baterka_delka, r= baterka_prumer/2, $fn=20);
      
}


module matka()
{    
translate([0,0,0]) 
    difference () 
    {
        
        cylinder (h = vyska_matky+1, r= sirka_matky/2+5, $fn=8);
        
        translate([0,0,1]) 
            cylinder (h = vyska_matky+0.31, r= (sirka_matky+0.2)/2, $fn=6);
        
        translate([0,0,-0.01]) 
            cylinder (h = vyska_sloupku, r= (prumer_sroubu+0.2)/2, $fn=40);
               
       
    
    }       
}