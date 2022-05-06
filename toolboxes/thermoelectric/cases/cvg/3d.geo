h=0.5;

Point(1) = {0,0,0,h};
Point(2) = {1,0,0,h};
Point(3) = {0,1,0,h};
Point(4) = {0,2,0,h};
Point(5) = {2,0,0,h};

Circle(1) = {2,1,3};
Line(2) = {3,4};
Circle(3) = {4,1,5};
Line(4) = {5,2};

Line Loop(1) = {1,2,3,4};
Plane Surface(1) = {1};

out[] = Extrude{0,0,5} {Surface{1};};

Physical Volume("omega") = {out[1]};
Physical Surface("top") = {out[0]};
Physical Surface("bottom") = {1};
Physical Surface("Rint") = {out[2]};
Physical Surface("V0") = {out[3]};
Physical Surface("Rext") = {out[4]};
Physical Surface("V1") = {out[5]};

