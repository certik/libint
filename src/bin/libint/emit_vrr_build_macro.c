#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "build_libint.h"

extern FILE *outfile, *vrr_header;
extern void punt(char *);
extern LibintParams_t Params;

static int hash(int a[2][3], int b[2]);
static void declare_localv();
static void define_localv();

static char **k1, **k2, **k3;

void emit_vrr_build_macro()
{
  int old_am = Params.old_am;
  int new_am = Params.opt_am;
  int max_class_size = Params.max_class_size;
  int am_to_inline = Params.max_am_to_inline_vrr_worker;

  FILE *code;
  int i, j, k, l, f;
  int a, b;
  int flag;
  int am[2][3];
  int am_in[2];
  int current_highest_am, to_inline;
  int nflip = 0;
  int t1, t2, t3, t4;
  int class_size;
  int type;
  int max1 = 0;
  int max2 = 0;
  int foo;
  int la, lc, lc_min, lc_max;
  int k1max, k2max, k3max;
  int split,num_subfunctions,subbatch_length;
  int curr_count,curr_subfunction;
  int FLOP_counter;
  static int io[] = {0,1,3,6,10,15,21,28,36,45,55,66,78,91,105,120,136,153};
  static char *k4[] = {"lpoz","lpon"};
  const char am_letter[] = "0pdfghiklmnoqrtuvwxyz";
  static const char *number[] = {"zero","one","two","three","four","five","six","seven","eight","nine","ten","eleven",
			       "twelve","thirteen","fourteen","fifteen","sixteen","seventeen","eighteen","nineteen","twenty"};
  static const char *k1_suff = "o2z";
  static const char *k2_suff = "o2zn";
  static const char *k3_suff = "o2n";
  char *code_name;
  char *function_name;
  char **subfunction_name;
  char *cpcommand;
  int errcod;

  k1 = (char **) malloc((new_am)*sizeof(char *));
  k2 = (char **) malloc((new_am)*sizeof(char *));
  k3 = (char **) malloc((new_am)*sizeof(char *));
  for(i=1;i<=new_am;i++) {
    j = strlen((void *)number[i]);
    k1[i-1] = (char*) malloc((4+j)*sizeof(char));
    k2[i-1] = (char*) malloc((5+j)*sizeof(char));
    k3[i-1] = (char*) malloc((4+j)*sizeof(char));
    strcpy(k1[i-1],(void *)number[i]);
    strcpy(k2[i-1],(void *)number[i]);
    strcpy(k3[i-1],(void *)number[i]);
    strcat(k1[i-1],(void *)k1_suff);
    strcat(k2[i-1],(void *)k2_suff);
    strcat(k3[i-1],(void *)k3_suff);
  }
  code_name = (char *) malloc(sizeof(char)*21);
  cpcommand = (char *) malloc(sizeof(char)*50);
  function_name = (char *) malloc(sizeof(char)*18);

  for(la=0;la<=new_am;la++) {
    lc_min = (la >= old_am + 1) ? 0 : old_am + 1;
    lc_max = new_am;
    for(lc=lc_min;lc<=lc_max;lc++) {

      /* Is this function to be made inline */
      current_highest_am = (la > lc) ? la : lc;
      to_inline = (current_highest_am <= am_to_inline) ? 1 : 0;
      if (!to_inline)
	continue;
      
      fprintf(outfile,"  AM_a = %c  AM_c = %c\n",am_letter[la],am_letter[lc]);
      am_in[0] = la;
      am_in[1] = lc;
      if (la == 0) {
	a = 1;
	k2max = la;
	k3max = lc - 1;
      }
      else {
	a = 0;
	k2max = lc;
	k1max = la - 1;
      }
      foo = 5;
      if(a==0) foo = 4;

      class_size = ((am_in[a]+1)*(am_in[a]+2)*(am_in[a^1]+1)*(am_in[a^1]+2))/4;

      /* If the routine has to be split AND inlined - the user probably doesn't know what he/she is doing */
      if (class_size > max_class_size)
	punt("MAX_CLASS_SIZE is too small with the given inlining thresholds");
      else {
	split = 0;
      }

      if(a==0) foo = 4;

      sprintf(function_name,"build_%c0%c0",am_letter[la],am_letter[lc]);
      sprintf(code_name,"build_%c0%c0.h",am_letter[la],am_letter[lc]);
      code = fopen(code_name,"w");

      /*target
        |I0[],I1[]
        |    |I2[],I3[]
        |    |    |   I4[]
        |    |    |    |     */
        t1 = t2 = t3 = t4 = 0;

      /* print local variable declarations */

      fprintf(code,"#ifndef _libint_%s\n",function_name);
      fprintf(code,"#define _libint_%s\n",function_name);
      fprintf(code,"  /* These machine-generated functions compute a quartet of (%cs|%cs) integrals */\n\n",am_letter[la],am_letter[lc]);

      fprintf(code,"#define _%s(Data, vp, I0, I1, I2, I3, I4)\\\n{\\\n",function_name);
      declare_localv(a,k1max,k2max,k3max,code);
      define_localv(a,foo,k1max,k2max,k3max,code);
      fprintf(code,"\\\n");

      FLOP_counter = 0;

      for(i = 0; i <= am_in[0]; i++){
	am[0][0] = am_in[0] - i;
	for(j = 0; j <= i; j++){
	  am[0][1] = i - j;
	  am[0][2] = j;

	  for(k = 0; k <= am_in[1]; k++){
	    am[1][0] = am_in[1] - k;
	    for(l = 0; l <= k; l++){
	      am[1][1] = k - l;
	      am[1][2] = l;

	      if(am[a][2]) b = 2;
	      if(am[a][1]) b = 1;
	      if(am[a][0]) b = 0;

          
	      am[a][b] = am[a][b] - 1;
	      am_in[a] = am_in[a] - 1;
	      t2 = hash(am,am_in);
	      fprintf(code, "*(target++) = U%d%d*i0[%d] + U%d%d*i1[%d]",
		      a*2, b, t2, foo, b , t2); 
              FLOP_counter += 3;
	      if(am[a][b]){
		am[a][b] = am[a][b] - 1;
		am_in[a] = am_in[a] - 1;
		t3 = hash(am,am_in);
		fprintf(code, "\\\n           + (%s)*(i2[%d] - (%s)*i3[%d])", 
			(a==0 ? k1[am[a][b]] : k3[am[a][b]]), 
			t3, (k4[a]), t3);
		max1 = (max1>am[a][b]+1) ? max1 : am[a][b]+1;
		am[a][b] = am[a][b] + 1;
		am_in[a] = am_in[a] + 1;
                FLOP_counter += 4;
	      }
	      if(am[a^1][b]){
		am[a^1][b] = am[a^1][b] - 1;
		am_in[a^1] = am_in[a^1] - 1;
		t4 = hash(am,am_in);
		fprintf(code, "\\\n           + (%s)*i4[%d]", k2[am[a^1][b]], t4);
		max2 = (max2>am[a^1][b]+1) ? max2 : am[a^1][b]+1;
		am[a^1][b] = am[a^1][b] + 1;
		am_in[a^1] = am_in[a^1] + 1;
                FLOP_counter += 2;
	      }
	      fprintf(code, ";\\\n");
	      am[a][b] = am[a][b] + 1;
	      am_in[a] = am_in[a] + 1;
		
	      t1++;
	      curr_count++;
	    }
	  }
	}
      }
      fprintf(code,"\\\n}\n");
      fprintf(code,"\n#endif\n"); /* end of #ifndef _libint_.... */
      fprintf(code,"/* Total number of FLOPs = %d */\n",FLOP_counter);
      fclose(code);
      printf("Done with %s\n",code_name);
    }
  }
  free(function_name);
  free(code_name);

  return;
}


int hash(a, b)
  int a[2][3];
  int b[2];
{
  int c[2] = {0,0};
  int i;
  static int io[] = {0,1,3,6,10,15,21,28,36,45,55,66,78,91,105,120,136,153};

  if(b[0]){
    i=b[0]-a[0][0];
    c[0]=i+io[i]-a[0][1];
    }
  if(b[1]){
    i=b[1]-a[1][0];
    c[1]=i+io[i]-a[1][1];
    }

  return c[0]*io[b[1]+1]+c[1];
}


void declare_localv(int a, int k1max, int k2max, int k3max, FILE *code)
{
  int i;

  fprintf(code,"  REALTYPE U00, U01, U02, U10, U11, U12, U20, U21, U22;\\\n");
  fprintf(code,"  REALTYPE U30, U31, U32, U40, U41, U42, U50, U51, U52;\\\n");
  fprintf(code,"  REALTYPE lpoz = Data->poz;\\\n  REALTYPE lpon = Data->pon;\\\n");
  for(i=0;i<k2max;i++)
    fprintf(code,"  REALTYPE %s;\\\n",k2[i]);
  if (a==0)
    for(i=0;i<k1max;i++)
      fprintf(code,"  REALTYPE %s;\\\n",k1[i]);
  else
    for(i=0;i<k3max;i++)
      fprintf(code,"  REALTYPE %s;\\\n",k3[i]);
  fprintf(code,"  REALTYPE *target = (vp);\\\n");
  fprintf(code,"  const REALTYPE *i0 = (I0);\\\n");
  fprintf(code,"  const REALTYPE *i1 = (I1);\\\n");
  fprintf(code,"  const REALTYPE *i2 = (I2);\\\n");
  fprintf(code,"  const REALTYPE *i3 = (I3);\\\n");
  fprintf(code,"  const REALTYPE *i4 = (I4);\\\n");
}

void define_localv(int a, int foo, int k1max, int k2max, int k3max, FILE *code)
{
  int i;
  
  for(i=0;i<k2max;i++)
    fprintf(code,"  %s = %.1lf*Data->oo2zn;\\\n",k2[i],(double)(i+1));
  if(a==0)
    for(i=0;i<k1max;i++)
      fprintf(code,"  %s = %.1lf*Data->oo2z;\\\n",k1[i],(double)(i+1));
  else
    for(i=0;i<k3max;i++)
      fprintf(code,"  %s = %.1lf*Data->oo2n;\\\n",k3[i],(double)(i+1));
  fprintf(code,"  U%d0 = Data->U[%d][0];\\\n", a*2, a*2);
  fprintf(code,"  U%d1 = Data->U[%d][1];\\\n", a*2, a*2);
  fprintf(code,"  U%d2 = Data->U[%d][2];\\\n", a*2, a*2);
  fprintf(code,"  U%d0 = Data->U[%d][0];\\\n", foo, foo);
  fprintf(code,"  U%d1 = Data->U[%d][1];\\\n", foo, foo);
  fprintf(code,"  U%d2 = Data->U[%d][2];\\\n\\\n", foo, foo);

}
