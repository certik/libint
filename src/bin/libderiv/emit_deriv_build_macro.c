#include <math.h>
#include <stdio.h>
#include <libderiv/build_libderiv.h>
#include <constants.h>

extern FILE *outfile, *deriv_header;
extern LibderivParams_t Params;

extern void punt(char *);

void emit_deriv_build_macro()
{
  int new_am = Params.new_am;
  int old_am = Params.old_am;
  int am_to_inline = Params.max_am_to_inline_deriv_worker;

  FILE *code;
  int p,q,r,s;
  int ax,ay,az,bx,by,bz,cx,cy,cz,dx,dy,dz;
  int i,j,nj,i_i0,i_i1;
  int k,l,nl,k_i0,k_i1;
  int i0_step,i1_step;
  int a, b;
  int flag;
  int am_in[2];
  int class_size;
  int la, lb;
  int ld, lc, ld_max;
  int xyz;
  int current_highest_am, to_inline;
  int errcod;
  char code_name[] = "deriv_build_X0_s.h";
  char function_name[] = "deriv_build_A0_s";

  for(la=0;la<=new_am+DERIV_LVL-1;la++) {

      /* Is this function to be made inline */
      current_highest_am = la;
      to_inline = (current_highest_am <= am_to_inline) ? 1 : 0;

      if (!to_inline)
	continue;

      function_name[15] = am_letter[la];
      code_name[15] = am_letter[la];
      
      /*---------------
	DR on center A
       ---------------*/

      function_name[12] = 'A';
      code_name[12] = 'A';
      
      /*--- vp = d(vp)/dai ---*/
      for(xyz = 0; xyz < 3; xyz++) {
	function_name[13] = cart_comp[xyz];
	code_name[13] = cart_comp[xyz];
	code = fopen(code_name,"w");

	fprintf(code,"#ifndef _libderiv_%s\n",function_name);
	fprintf(code,"#define _libderiv_%s\n\n",function_name);
	fprintf(code,"#define %s(Data, bcd_num, vp, I0, I1)\\\n{\\\n",function_name);
	fprintf(code,"  const double twotzeta = Data->twozeta_a;\\\n");
	fprintf(code,"  const double *i0, *i1;\\\n");
	fprintf(code,"  double *target = (vp);\\\n");
	fprintf(code,"  int bcd;\\\n\\\n");

	  i0_step = (la+2)*(la+3)/2;
	  i1_step = la*(la+1)/2;

	  for(p = 0; p <= la; p++) {
	    ax = la - p;
	    for(q = 0; q <= p; q++) {
	      ay = p - q;
	      az = q;
	  
	      if (xyz == 0) { /* build along x */
		i_i0 = io(p)-p+q-1;
		if (i_i0)
		  fprintf(code,"  i0 = I0 + %d*bcd_num;\\\n",i_i0);
		else
		  fprintf(code,"  i0 = I0;\\\n");
		if (ax) {
		  i_i1 = io(p)-p+q-1;
		  if (i_i1)
		    fprintf(code,"  i1 = I1 + %d*bcd_num;\\\n",i_i1);
		  else
		    fprintf(code,"  i1 = I1;\\\n");
		}
		fprintf(code,"  for(bcd=0;bcd<bcd_num;bcd++)\\\n");
		fprintf(code,"    *(target++) = twotzeta*(*(i0++)) ");
		if (ax)
		  fprintf(code,"- %lf*(*(i1++));\\\n",(double)ax);
		else
		  fprintf(code,";\\\n");
	      }
	      else if (xyz == 1) { /* build along y */
		i_i0 = io(p+1)-p+q-2;
		if (i_i0)
		  fprintf(code,"  i0 = I0 + %d*bcd_num;\\\n",i_i0);
		else
		  fprintf(code,"  i0 = I0;\\\n");
		if (ay) {
		  i_i1 = io(p-1)-p+q;
		  if (i_i1)
		    fprintf(code,"  i1 = I1 + %d*bcd_num;\\\n",i_i1);
		  else
		    fprintf(code,"  i1 = I1;\\\n");
		}
		fprintf(code,"  for(bcd=0;bcd<bcd_num;bcd++)\\\n");
		fprintf(code,"    *(target++) = twotzeta*(*(i0++)) ");
		if (ay)
		  fprintf(code,"- %lf*(*(i1++));\\\n",(double)ay);
		else
		  fprintf(code,";\\\n");
	      }
	      else { /* build along z */
		i_i0 = io(p+1)-p+q-1;
		if (i_i0)
		  fprintf(code,"  i0 = I0 + %d*bcd_num;\\\n",i_i0);
		else
		  fprintf(code,"  i0 = I0;\\\n");
		if (az) {
		  i_i1 = io(p-1)-p+q-1;
		  if (i_i1)
		    fprintf(code,"  i1 = I1 + %d*bcd_num;\\\n",i_i1);
		  else
		    fprintf(code,"  i1 = I1;\\\n");
		}
		fprintf(code,"  for(bcd=0;bcd<bcd_num;bcd++)\\\n");
		fprintf(code,"    *(target++) = twotzeta*(*(i0++)) ");
		if (az)
		  fprintf(code,"- %lf*(*(i1++));\\\n",(double)az);
		else
		  fprintf(code,";\\\n");
	      }
	    }
	  }
	  fprintf(code,"}\n");
	  fprintf(code,"\n#endif\n"); /* end of #ifndef _libderiv_.... */
	  fclose(code);
	  printf("Done with %s\n",code_name);
      }

	
      /*---------------
	DR on center B
       ---------------*/
      lb = la;
      function_name[12] = 'B';
      code_name[12] = 'B';
      
      /*--- vp = d(vp)/dbi ---*/
      for(xyz = 0; xyz < 3; xyz++) {
	function_name[13] = cart_comp[xyz];
	code_name[13] = cart_comp[xyz];
	code = fopen(code_name,"w");

	fprintf(code,"#ifndef _libderiv_%s\n",function_name);
	fprintf(code,"#define _libderiv_%s\n",function_name);
	fprintf(code,"#define %s(Data, a_num, cd_num, vp, I0, I1)\\\n{\\\n",
		function_name);
	fprintf(code,"  const double twotzeta = Data->twozeta_b;\\\n");
	fprintf(code,"  const double *i0, *i1;\\\n");
	fprintf(code,"  double *target = (vp);\\\n");
	fprintf(code,"  double *II0 = (I0);\\\n");
	fprintf(code,"  double *II1 = (I1);\\\n");
	fprintf(code,"  int a,cd;\\\n\\\n");
	i0_step = (lb+2)*(lb+3)/2;
	i1_step = lb*(lb+1)/2;

	  fprintf(code,"  for(a=0;a<a_num;a++) {\\\n");
	  for(p = 0; p <= lb; p++) {
	    bx = lb - p;
	    for(q = 0; q <= p; q++) {
	      by = p - q;
	      bz = q;
	  
	      if (xyz == 0) { /* build along x */
		i_i0 = io(p)-p+q-1;
		if (i_i0)
		  fprintf(code,"  i0 = II0 + %d*cd_num;\\\n",i_i0);
		else
		  fprintf(code,"  i0 = II0;\\\n");
		if (bx) {
		  i_i1 = io(p)-p+q-1;
		  if (i_i1)
		      fprintf(code,"  i1 = II1 + %d*cd_num;\\\n",i_i1);
		  else
		    fprintf(code,"  i1 = II1;\\\n");
		}
		fprintf(code,"  for(cd=0;cd<cd_num;cd++)\\\n");
		fprintf(code,"    *(target++) = twotzeta*(*(i0++)) ");
		if (bx)
		  fprintf(code,"- %lf*(*(i1++));\\\n",(double)bx);
		else
		  fprintf(code,";\\\n");
	      }
	      else if (xyz == 1) { /* build along y */
		i_i0 = io(p+1)-p+q-2;
		if (i_i0)
		  fprintf(code,"  i0 = II0 + %d*cd_num;\\\n",i_i0);
		else
		  fprintf(code,"  i0 = II0;\\\n");
		if (by) {
		  i_i1 = io(p-1)-p+q;
		  if (i_i1)
		    fprintf(code,"  i1 = II1 + %d*cd_num;\\\n",i_i1);
		  else
		    fprintf(code,"  i1 = II1;\\\n");
		}
		fprintf(code,"  for(cd=0;cd<cd_num;cd++)\\\n");
		fprintf(code,"    *(target++) = twotzeta*(*(i0++)) ");
		if (by)
		  fprintf(code,"- %lf*(*(i1++));\\\n",(double)by);
		else
		  fprintf(code,";\\\n");
	      }
	      else { /* build along z */
		i_i0 = io(p+1)-p+q-1;
		if (i_i0)
		  fprintf(code,"  i0 = II0 + %d*cd_num;\\\n",i_i0);
		else
		  fprintf(code,"  i0 = II0;\\\n");
		if (bz) {
		  i_i1 = io(p-1)-p+q-1;
		  if (i_i1)
		    fprintf(code,"  i1 = II1 + %d*cd_num;\\\n",i_i1);
		  else
		    fprintf(code,"  i1 = II1;\\\n");
		}
		fprintf(code,"  for(cd=0;cd<cd_num;cd++)\\\n");
		fprintf(code,"    *(target++) = twotzeta*(*(i0++)) ");
		if (bz)
		  fprintf(code,"- %lf*(*(i1++));\\\n",(double)bz);
		else
		  fprintf(code,";\\\n");
	      }
	    }
	  }
	  fprintf(code,"  II0 += %d*cd_num;  II1 += %d*cd_num;\\\n",i0_step,i1_step);
	  fprintf(code,"  }\\\n}\n");
	  fprintf(code,"\n#endif\n"); /* end of #ifndef _libderiv_.... */
	  fclose(code);
	  printf("Done with %s\n",code_name);
      }


      /*---------------
	DR on center C
       ---------------*/
      lc = la;
      function_name[12] = 'C';
      code_name[12] = 'C';
      
      /*--- vp = d(vp)/dci ---*/
      for(xyz = 0; xyz < 3; xyz++) {
	function_name[13] = cart_comp[xyz];
	code_name[13] = cart_comp[xyz];
	code = fopen(code_name,"w");

	fprintf(code,"#ifndef _libderiv_%s\n",function_name);
	fprintf(code,"#define _libderiv_%s\n",function_name);
	fprintf(code,"#define %s(Data, ab_num, d_num, vp, I0, I1)\\\n{\\\n",
		function_name);
	fprintf(code,"  const double twotzeta = Data->twozeta_c;\\\n");
	fprintf(code,"  const double *i0, *i1;\\\n");
	fprintf(code,"  double *target = (vp);\\\n");
	fprintf(code,"  double *II0 = (I0);\\\n");
	fprintf(code,"  double *II1 = (I1);\\\n");
	fprintf(code,"  int ab,d;\\\n\\\n");
	i0_step = (lc+2)*(lc+3)/2;
	i1_step = lc*(lc+1)/2;

	  fprintf(code,"  for(ab=0;ab<ab_num;ab++) {\\\n");
	  for(p = 0; p <= lc; p++) {
	    cx = lb - p;
	    for(q = 0; q <= p; q++) {
	      cy = p - q;
	      cz = q;
	  
	      if (xyz == 0) { /* build along x */
		i_i0 = io(p)-p+q-1;
		if (i_i0)
		  fprintf(code,"  i0 = II0 + %d*d_num;\\\n",i_i0);
		else
		  fprintf(code,"  i0 = II0;\\\n");
		if (cx) {
		  i_i1 = io(p)-p+q-1;
		  if (i_i1)
		      fprintf(code,"  i1 = II1 + %d*d_num;\\\n",i_i1);
		  else
		    fprintf(code,"  i1 = II1;\\\n");
		}
		fprintf(code,"  for(d=0;d<d_num;d++)\\\n");
		fprintf(code,"    *(target++) = twotzeta*(*(i0++)) ");
		if (cx)
		  fprintf(code,"- %lf*(*(i1++));\\\n",(double)cx);
		else
		  fprintf(code,";\\\n");
	      }
	      else if (xyz == 1) { /* build along y */
		i_i0 = io(p+1)-p+q-2;
		if (i_i0)
		  fprintf(code,"  i0 = II0 + %d*d_num;\\\n",i_i0);
		else
		  fprintf(code,"  i0 = II0;\\\n");
		if (cy) {
		  i_i1 = io(p-1)-p+q;
		  if (i_i1)
		    fprintf(code,"  i1 = II1 + %d*d_num;\\\n",i_i1);
		  else
		    fprintf(code,"  i1 = II1;\\\n");
		}
		fprintf(code,"  for(d=0;d<d_num;d++)\\\n");
		fprintf(code,"    *(target++) = twotzeta*(*(i0++)) ");
		if (cy)
		  fprintf(code,"- %lf*(*(i1++));\\\n",(double)cy);
		else
		  fprintf(code,";\\\n");
	      }
	      else { /* build along z */
		i_i0 = io(p+1)-p+q-1;
		if (i_i0)
		  fprintf(code,"  i0 = II0 + %d*d_num;\\\n",i_i0);
		else
		  fprintf(code,"  i0 = II0;\\\n");
		if (cz) {
		  i_i1 = io(p-1)-p+q-1;
		  if (i_i1)
		    fprintf(code,"  i1 = II1 + %d*d_num;\\\n",i_i1);
		  else
		    fprintf(code,"  i1 = II1;\\\n");
		}
		fprintf(code,"  for(d=0;d<d_num;d++)\\\n");
		fprintf(code,"    *(target++) = twotzeta*(*(i0++)) ");
		if (cz)
		  fprintf(code,"- %lf*(*(i1++));\\\n",(double)cz);
		else
		  fprintf(code,";\\\n");
	      }
	    }
	  }
	  fprintf(code,"  II0 += %d*d_num;  II1 += %d*d_num;\\\n",i0_step,i1_step);
	fprintf(code,"  }\\\n}\n");
	fprintf(code,"\n#endif\n"); /* end of #ifndef _libderiv_.... */
	fclose(code);
	printf("Done with %s\n",code_name);
      }

      /*---------------
	DR on center D
       ---------------*/
      ld = la;
      function_name[12] = 'D';
      code_name[12] = 'D';
      
      /*--- vp = d(vp)/ddi ---*/
      for(xyz = 0; xyz < 3; xyz++) {
	function_name[13] = cart_comp[xyz];
	code_name[13] = cart_comp[xyz];
	code = fopen(code_name,"w");

	fprintf(code,"#ifndef _libderiv_%s\n",function_name);
	fprintf(code,"#define _libderiv_%s\n",function_name);
	fprintf(code,"#define %s(Data, abc_num, vp, I0, I1)\\\n{\\\n",
		function_name);
	fprintf(code,"  const double twotzeta = Data->twozeta_d;\\\n");
	fprintf(code,"  double *target = (vp);\\\n");
	fprintf(code,"  double *II0 = (I0);\\\n");
	fprintf(code,"  double *II1 = (I1);\\\n");
	fprintf(code,"  int abc;\\\n\\\n");
	i0_step = (ld+2)*(ld+3)/2;
	i1_step = ld*(ld+1)/2;

	  fprintf(code,"  for(abc=0;abc<abc_num;abc++) {\\\n");
	  for(p = 0; p <= lc; p++) {
	    dx = lb - p;
	    for(q = 0; q <= p; q++) {
	      dy = p - q;
	      dz = q;
	  
	      if (xyz == 0) { /* build along x */
		i_i0 = io(p)-p+q-1;
		if (dx) {
		  i_i1 = io(p)-p+q-1;
		}
		fprintf(code,"    *(target++) = twotzeta*II0[%d] ", i_i0);
		if (dx)
		  fprintf(code,"- %lf*II1[%d];\\\n", (double)dx, i_i1);
		else
		  fprintf(code,";\\\n");
	      }
	      else if (xyz == 1) { /* build along y */
		i_i0 = io(p+1)-p+q-2;
		if (dy) {
		  i_i1 = io(p-1)-p+q;
		}
		fprintf(code,"    *(target++) = twotzeta*II0[%d] ", i_i0);
		if (dy)
		  fprintf(code,"- %lf*II1[%d];\\\n", (double)dy, i_i1);
		else
		  fprintf(code,";\\\n");
	      }
	      else { /* build along z */
		i_i0 = io(p+1)-p+q-1;
		if (dz) {
		  i_i1 = io(p-1)-p+q-1;
		}
		fprintf(code,"    *(target++) = twotzeta*II0[%d] ", i_i0);
		if (dz)
		  fprintf(code,"- %lf*II1[%d];\\\n", (double)dz, i_i1);
		else
		  fprintf(code,";\\\n");
	      }
	    }
	  }
	  fprintf(code,"  II0 += %d;  II1 += %d;\\\n",i0_step,i1_step);
	fprintf(code,"  }\\\n}\n");
	fprintf(code,"\n#endif\n"); /* end of #ifndef _libderiv_.... */
	fclose(code);
	printf("Done with %s\n",code_name);
      }  
      
  }

  return;
}



