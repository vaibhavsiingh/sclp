proc main begin
temp0 = a > 20
temp1 = !temp0
temp2 = !temp1
if (temp2) goto Label1
a = 30
goto Label0
Label1:
a = 40
Label0:
Label2:
temp3 = a > 30
temp4 = !temp3
if (temp4) goto Label3
a = 40
goto Label2
Label3:
temp5 = -30
a = temp5
temp6 = -a
temp7 = temp6 > 30
temp8 = !temp7
if (temp8) goto Label5
a = 50
goto Label4
Label5:
Label4:
proc main end

