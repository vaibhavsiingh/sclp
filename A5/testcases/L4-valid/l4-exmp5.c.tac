proc main begin
read local
global2 = local
global1 = 5
temp0 = global2 > global1
temp1 = local - 2
temp2 = temp1 != 0
temp3 = temp0 && temp2
temp4 = !temp3
if (temp4) goto Label0
stemp0 = 1
goto Label1
Label0:
stemp0 = 0
Label1:
ans = stemp0
temp5 = ans == 1
temp9 = !temp5
if (temp9) goto Label5
Label2:
temp6 = global2 > 0
temp8 = !temp6
if (temp8) goto Label3
print global2
temp7 = global2 - 1
global2 = temp7
goto Label2
Label3:
goto Label4
Label5:
Label6:
print global2
temp10 = global2 + 1
global2 = temp10
temp11 = global2 < 10
if (temp11) goto Label6
Label4:
proc main end

