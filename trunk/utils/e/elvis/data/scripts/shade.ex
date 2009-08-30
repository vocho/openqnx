"This script defines the hlobjectN colors as being shades of gray
set hllayers=9
switch current("background")
case light {
 for i (1 ... hllayers)
 do eval color hlobject(i) on gray(90 - 3*i)
}
case dark {
 for i (1 ... hllayers)
 do eval color hlobject(i) on gray(10 + 4*i)
}
