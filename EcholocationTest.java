/*--------------------------------------------------------------------------------------*/
/*  EcholocationTest.java  -  Description                                               */
/*                                                                                      */
/*--------------------------------------------------------------------------------------*/
/*  Author: Enoch Wong                                                                  */
/*  Date:                                                                               */
/*--------------------------------------------------------------------------------------*/
/*  Input:                                                                              */
/*  Output:                                                                             */
/*--------------------------------------------------------------------------------------*/

import java.io.*;
import java.util.*;
import java.text.*;

public class EcholocationTest
{

    public static void main (String str[]) throws IOException
    {
	BufferedReader stdin = new BufferedReader (new InputStreamReader (System.in));

	System.out.print ("Enter D1: ");
	double D1 = Double.parseDouble (stdin.readLine ());

	System.out.print ("Enter D2: ");
	double D2 = Double.parseDouble (stdin.readLine ());

	System.out.print ("Enter D3: ");
	double D3 = Double.parseDouble (stdin.readLine ());

	System.out.print ("Enter D4: ");
	double D4 = Double.parseDouble (stdin.readLine ());

	System.out.println ();
	calculate (D1, D2, D3, D4);
    }


    static void calculate (double D1, double D2, double D3, double D4)
    {
	DecimalFormat df = new DecimalFormat ("#.##");
	int length = 100;

	double s1 = (D1 + D2 + length) / 2;
	double area1 = Math.sqrt (s1 * (s1 - D1) * (s1 - D2) * (s1 - length));
	System.out.println ("Area 1: " + area1);
	double M1 = 2 * area1 / length;
	System.out.println ("Midpoint 1: " + M1);
	double x = Math.sqrt (Math.pow (D1, 2) - Math.pow (M1, 2));
	System.out.println ();

	double s2 = (D3 + D4 + length) / 2;
	double area2 = Math.sqrt (s2 * (s2 - D3) * (s2 - D4) * (s2 - length));
	System.out.println ("Area 2: " + area2);
	double M2 = 2 * area2 / length;
	System.out.println ("Midpoint 2: " + M2);
	double y = Math.sqrt (Math.pow (D3, 2) - Math.pow (M2, 2));
	System.out.println ();

	double z = 100 - Math.sqrt (Math.pow (M2, 2) - Math.pow (length - x + 50, 2));

	System.out.println ("X VALUE: " + df.format (x));
	System.out.println ("Y VALUE: " + df.format (y));
	System.out.println ("Z VALUE: " + df.format (z));
    }
}


