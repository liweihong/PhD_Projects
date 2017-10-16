function [R , R_inv] = matrix_value(dest_value, mimio_value, num)

dest_value = dest_value';
mimio_value = mimio_value';

X0 = mimio_value(:,1);
Y0 = mimio_value(:,2);
X = dest_value(:,1);
Y = dest_value(:,2);
       
for i = 1 : num
	B(i*2 - 1,1) = X(i); 
	B(i*2,1) = Y(i);
end;   
       
for i = 1 : num
	A(i*2 - 1, 1) = X0(i);
	A(i*2, 1) = 0;
	A(i*2 - 1, 2) = Y0(i);
	A(i*2, 2) = 0;
	A(i*2 - 1, 3) = 1;
	A(i*2, 3) = 0;
	A(i*2 - 1, 4) = 0;
	A(i*2, 4) = X0(i);
	A(i*2 - 1, 5) = 0;
	A(i*2, 5) = Y0(i);
	A(i*2 - 1, 6) = 0;
	A(i*2, 6) = 1;
	A(i*2 - 1, 7) = -X(i)*X0(i);
	A(i*2, 7) = -X0(i)*Y(i);
	A(i*2 - 1, 8) = -Y0(i)*X(i);
	A(i*2, 8) = -Y(i)*Y0(i);
end;

R = A\B;
T = R;

R = [R',1];
R = reshape(R,3,3);
R = inv(R);
R = reshape(R,9,1);
R = R/R(9,1);

R_inv = R
R = T
