void main()
{
    float marks;
    string grade;

    print "Enter your percentage ";
    read marks;

    grade = marks < 40.0 ? "F" : marks >=40 ? "A" : "B";
    print "The grade obtained is: ";
    print grade;

}