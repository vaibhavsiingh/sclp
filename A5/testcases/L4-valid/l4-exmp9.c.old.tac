proc main begin
temp0 = a > 20
temp1 = !temp0
if (temp1) goto Label1
b = 20
goto Label0
Label1:
c = 30
Label0:
Label2:
temp2 = b > 20
temp3 = c < 30
temp4 = temp2 && temp3
temp6 = !temp4
if (temp6) goto Label3
temp5 = b + 1
b = temp5
goto Label2
Label3:
Label4:
temp7 = c + 2
c = temp7
temp8 = c > 30
if (temp8) goto Label4
proc main end

