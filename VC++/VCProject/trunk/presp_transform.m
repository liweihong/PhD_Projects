function [R , R1] = presp_transform(Coorb, Coora, B, pointnumber)

   
for cnt=1 : pointnumber,
    AA(cnt, 1) = Coora(cnt, 1) ;
    AA(cnt, 2) = Coora(cnt, 2) ;
    AA(cnt, 3) = 1;
    AA(cnt, 4) = 0;
    AA(cnt, 5) = 0;
    AA(cnt, 6) = 0;
    AA(cnt, 7) = -(Coora(cnt, 1))*(Coorb(cnt,1));
    AA(cnt, 8) = -(Coora(cnt, 2))*(Coorb(cnt,1));
end;

for cnt= pointnumber+1 : 2*pointnumber,
    AA(cnt, 1) = 0;
    AA(cnt, 2) = 0;
    AA(cnt, 3) = 0;
    AA(cnt, 4) = Coora(cnt-pointnumber, 1) ;
    AA(cnt, 5) = Coora(cnt-pointnumber, 2) ;
    AA(cnt, 6) = 1;
    AA(cnt, 7) = -(Coora(cnt-pointnumber, 1))*(Coorb(cnt-pointnumber, 2));
    AA(cnt, 8) = -(Coora(cnt-pointnumber, 2))*(Coorb(cnt-pointnumber, 2));
end;

R = ones(8,1);
A = AA' * AA;
[U,S,V] = svd(A);
R = V* inv (S) * U' * AA' * B;
T = R;

R = [R',1];
R = reshape(R,3,3);
R = inv(R);
R = reshape(R,9,1);
R = R/R(9,1);

R1 = R
R = T
