/*
 Copyright  ETIS — ENSEA, Université de Cergy-Pontoise, CNRS (1991-2014)
 promethe@ensea.fr

 Authors: P. Andry, J.C. Baccon, D. Bailly, A. Blanchard, S. Boucena, A. Chatty, N. Cuperlier, P. Delarboulas, P. Gaussier,
 C. Giovannangeli, C. Grand, L. Hafemeister, C. Hasson, S.K. Hasnain, S. Hanoune, J. Hirel, A. Jauffret, C. Joulain, A. Karaouzène,
 M. Lagarde, S. Leprêtre, M. Maillard, B. Miramond, S. Moga, G. Mostafaoui, A. Pitti, K. Prepin, M. Quoy, A. de Rengervé, A. Revel ...

 See more details and updates in the file AUTHORS

 This software is a computer program whose purpose is to simulate neural networks and control robots or simulations.
 This software is governed by the CeCILL v2.1 license under French law and abiding by the rules of distribution of free software.
 You can use, modify and/ or redistribute the software under the terms of the CeCILL v2.1 license as circulated by CEA, CNRS and INRIA at the following URL "http://www.cecill.info".
 As a counterpart to the access to the source code and  rights to copy, modify and redistribute granted by the license,
 users are provided only with a limited warranty and the software's author, the holder of the economic rights,  and the successive licensors have only limited liability.
 In this respect, the user's attention is drawn to the risks associated with loading, using, modifying and/or developing or reproducing the software by the user in light of its specific status of free software,
 that may mean  that it is complicated to manipulate, and that also therefore means that it is reserved for developers and experienced professionals having in-depth computer knowledge.
 Users are therefore encouraged to load and test the software's suitability as regards their requirements in conditions enabling the security of their systems and/or data to be ensured
 and, more generally, to use and operate it in the same conditions as regards security.
 The fact that you are presently reading this means that you have had knowledge of the CeCILL v2.1 license and that you accept its terms.
 */
/**
 \ingroup libSensors
 \defgroup f_optical_flow f_optical_flow

 \brief calcul du flot optique

 \section Modified
 \author Syed Khursheed HASNAIN.
 \date Feb. 2012

 \details

 Introduce an algorthmic link named "enable". It is an optional link. If it is not used optical flow works as previous. But if enable link is used than (enable = 0) resets the optical flow output and (enable = 1) works normally.

 \section Option
 - 	enable : optional, this link will reset the optical flow if the input neuron is 0.

 \file
 \ingroup f_optical_flow f_optical_flow

 */

/* #define DEBUG */

#include <libx.h>
#include <Struct/prom_images_struct.h>

#include <stdlib.h>

#include <Kernel_Function/find_input_link.h>
#include <Kernel_Function/prom_getopt.h>
#include <public_tools/Vision.h>

typedef struct MyData_f_optical_flow
{
   int Gpe_it;
   int Gpe_it1;
   int nb_iterations;
   float alpha;
   prom_images_struct *prom_It, *prom_It1;
   prom_images_struct *prom_M_u, *prom_M_v, *prom_output, *prom_U, *prom_V;
   prom_images_struct *prom_Ix, *prom_Iy, *prom_Idt;

   type_neurone *neurons_of_enable; /* ****** modified*/

   unsigned char *It, *It1;
   float *U, *V;
   float *Ix, *Iy, *Idt;
   float *iv;
   /* unsigned char *iv; */ /* mdif PG: pour supprimer warning */
} MyData_f_optical_flow;

void function_optical_flow(int Gpe)
{
   int l, i, j, p, q;
   int iter;
   int largeur = 0, hauteur = 0;
   int Gpe_it = -1, Gpe_it1 = -1;
   int nb_iterations = 1;
   float alpha = 1.;

   int enabled; /* ****** modified*/
   type_liaison *link; /* ****** modified*/
   type_groupe *group; /* ****** modified*/

   unsigned char *It = NULL, *It1 = NULL;
   float M_u, M_v;
   float *U = NULL, *V = NULL;

   float *Ix = NULL, *Iy = NULL, *Idt = NULL;
   float div_coeff, div_coeff2;
   char resultat[256];
   MyData_f_optical_flow *my_data;

   float pt_c, pt_d, pt_b, pt_bd;
   float pt1_c, pt1_d, pt1_b, pt1_bd;
   float v_Ix, v_Iy;
   prom_images_struct *prom_It = NULL, *prom_It1 = NULL;
   prom_images_struct *prom_output = NULL;
   prom_images_struct *prom_Ix = NULL, *prom_Iy = NULL, *prom_Idt = NULL;

   group = &def_groupe[Gpe]; /* ****** modified*/

   /* ****** modified  shifted start */

   my_data = (MyData_f_optical_flow *) malloc(sizeof(MyData_f_optical_flow));
   if (my_data == NULL)
   {
      EXIT_ON_ERROR("erreur malloc dans f_optical_flow\n");
   }

   /* ****** modified shifted stop    */

   my_data->neurons_of_enable = NULL; /* ****** modified*/

   /* Recherche des deux gpes d'entreee  */
   if (def_groupe[Gpe].data == NULL)
   {
      i = 0;
      l = find_input_link(Gpe, i);
      while (l != -1)
      {
         link = &liaison[l]; /* ****  modified */

         dprints("lien %d: %s--\n", i, liaison[l].nom);

         if (prom_getopt(liaison[l].nom, "-enable", resultat) == 1)
         {
            my_data->neurons_of_enable = &neurone[def_groupe[link->depart].premier_ele]; /* read the input value */

         }
         if (prom_getopt(liaison[l].nom, "-I", resultat) == 1)
         {
            Gpe_it = liaison[l].depart;
            prom_It = (prom_images_struct *) def_groupe[Gpe_it].ext;
         }

         if (prom_getopt(liaison[l].nom, "-N", resultat) == 2)
         {
            nb_iterations = atoi(resultat);
         }
         if (prom_getopt(liaison[l].nom, "-alpha", resultat) == 2)
         {
            alpha = atof(resultat);
         }

         i++;
         l = find_input_link(Gpe, i);
      }
      if (Gpe_it == -1)
      {
         EXIT_ON_ERROR("manque un groupe dans f_optical_flow %d \n", Gpe_it);
      }

      /* ****** modified   shfited from here  (memory allocation)  */

      /* Test pour voir si les images sont vides */
      if (prom_It == NULL)
      {
         EXIT_ON_ERROR("Probleme (f_optical_flow) : il n'y pas d'image dans le groupe %i \n", Gpe_it);
      }

      largeur = prom_It->sx;
      hauteur = prom_It->sy;

      /* Allocation memoire pour l'image resultat */
      if (def_groupe[Gpe].data == NULL)
      {
         prom_It1 = calloc_prom_image(1, largeur, hauteur, 1); /* copie z-1 image unsigned char en entree It*/
         It1 = my_data->It1 = prom_It1->images_table[0];
         memset(It1, 0, largeur * hauteur * sizeof(unsigned char));

         prom_Ix = calloc_prom_image(1, largeur, hauteur, 4);
         prom_Iy = calloc_prom_image(1, largeur, hauteur, 4);
         prom_Idt = calloc_prom_image(1, largeur, hauteur, 4);

         prom_output = calloc_prom_image(2, largeur, hauteur, 4);
         def_groupe[Gpe].ext = prom_output;
      }

      my_data->Gpe_it = Gpe_it;
      my_data->Gpe_it1 = Gpe_it1;
      my_data->prom_It = prom_It;
      my_data->prom_It1 = prom_It1;
      my_data->alpha = alpha;
      my_data->nb_iterations = nb_iterations;

      It = my_data->It = prom_It->images_table[0];
      It1 = my_data->It1 = prom_It1->images_table[0];

      U = my_data->U = (float *) prom_output->images_table[0];
      V = my_data->V = (float *) prom_output->images_table[1];
      memset((char*) U, 0, largeur * hauteur * sizeof(float));
      memset((char*) V, 0, largeur * hauteur * sizeof(float));
      Ix = my_data->Ix = (float *) prom_Ix->images_table[0];
      Iy = my_data->Iy = (float *) prom_Iy->images_table[0];
      Idt = my_data->Idt = (float *) prom_Idt->images_table[0];

      def_groupe[Gpe].data = (MyData_f_optical_flow *) my_data;
   }
   else
   {
      my_data = (MyData_f_optical_flow *) (def_groupe[Gpe].data);
      Gpe_it = my_data->Gpe_it;
      Gpe_it1 = my_data->Gpe_it1;
      It = my_data->It;
      It1 = my_data->It1;

      U = my_data->U;
      V = my_data->V;
      Ix = my_data->Ix;
      Iy = my_data->Iy;
      Idt = my_data->Idt;

      prom_It = my_data->prom_It;
      prom_It1 = my_data->prom_It1;
      prom_Ix = my_data->prom_Ix;
      prom_Iy = my_data->prom_Iy;
      prom_Idt = my_data->prom_Idt;

      alpha = my_data->alpha;
      nb_iterations = my_data->nb_iterations;

      largeur = prom_It->sx;
      hauteur = prom_It->sy;
   }

   /*dprints("optical_flow nb_iterations= %d alpha=%f (%d %d)\n",nb_iterations,alpha,largeur,hauteur);*/

   if (my_data->neurons_of_enable == NULL) enabled = 1; /* ****** modified*/
   else if (my_data->neurons_of_enable[0].s1 >= 0.5) enabled = 1; /* ****** modified*/
   else enabled = 0; /* ****** modified*/
   /*dprints("valeur d'enable : %d\n",enabled);*/
   if (enabled) /* ****** modified*/
   {
      //dprints("-YES-");
      /* Calcul des Dérivées en X,Y et T */

      div_coeff = 1. / 4.;
      for (j = hauteur - 2; j--;)
      {
         /*	printf("j=%d : \n",j);*/
         q = (j + 1) * largeur + 1;
         for (i = largeur - 2; i--;)
         {
            /*	printf("i=%d ",i);*/
            p = i + q;
            pt_c = It[p]; /* Pixel central dans l'image au temps t         */
            pt_d = It[p + 1]; /* Pixel à droite dans l'image au temps t        */
            pt_b = It[p + largeur]; /* Pixel en bas dans l'image au temps t          */
            pt_bd = It[p + 1 + largeur]; /* Pixel en bas à droite dans l'image au temps t */

            pt1_c = It1[p]; /* Pixel central dans l'image au temps t-1         */
            pt1_d = It1[p + 1]; /* Pixel à droite dans l'image au temps t-1        */
            pt1_b = It1[p + largeur]; /* Pixel en bas dans l'image au temps t-1          */
            pt1_bd = It1[p + 1 + largeur]; /* Pixel en bas à droite dans l'image au temps t-1 */

            /*Calcul de la dérivée en Y (erreur c'est X) */
            Iy[p] = ((pt_bd + pt_b - pt_c - pt_d) + (pt1_bd + pt1_b - pt1_c - pt1_d)) * div_coeff;
            /****************************/

            /*Calcul de la dérivée en X (idem c'est Y) */
            Ix[p] = ((pt_d + pt_bd - pt_c - pt_b) + (pt1_d + pt1_bd - pt1_c - pt1_b)) * div_coeff;

            /****************************/
            /*Calcul de la dérivée en T */
            Idt[p] = ((pt1_c + pt1_b + pt1_d + pt1_bd) - (pt_c + pt_b + pt_d + pt_bd)) * div_coeff;
            /*	 if(j%5==0 && i%5==0) printf("%f %f %f,",Iy[p],Ix[p],Idt[p]);*/
            /****************************/
         }
      }
      /***********************************/

      /* DEBUT DES ITERATIONS */
      /*	memset((char*)U,0, largeur*hauteur*sizeof(float)	);
       memset((char*)V,0, largeur*hauteur*sizeof(float)	);	*/
      div_coeff = 1. / 12.;
      div_coeff2 = 1. / 6.;
      for (iter = nb_iterations; iter--;) /* on s'arrete a 0 */
      {
         /* printf("Iteration numéro  : %d \n",iter); */

         /***********Boucle de parcours des images*************/
         /*ATTENTION  : les première et dernières lignes et colonnes ne sont pas traitée */

         /*Copie des matrices tempons pour la prochaine itération */
         /*copy_matrix(M_u,nrl,nrh,ncl,nch,Tmp_u,nrl,nrh,ncl,nch);*/
         /*copy_matrix(M_v,nrl,nrh,ncl,nch,Tmp_v,nrl,nrh,ncl,nch);*/
         /*************************************************************************/

         for (j = hauteur - 2; j--;)
         {
            q = (j + 1) * largeur + 1;
            for (i = largeur - 2; i--;) /* for (i=1; i<largeur-1; i++) */
            {
               p = i + q;
               /*Calcul de la moyenne des vecteurs en X */
               /* note PG: M_u et M_v ont ils besoin d'etre sauvegarde dans des vecteurs??? */
               M_u = (U[p - largeur - 1] + U[p - largeur + 1] + U[p + largeur - 1] + U[p + largeur + 1]) * div_coeff + (U[p - largeur] + U[p - 1] + U[p + 1] + U[p + largeur]) * div_coeff2;

               /*Calcul de la moyenne des vecteurs en Y */
               M_v = (V[p - largeur - 1] + V[p - largeur + 1] + V[p + largeur - 1] + V[p + largeur + 1]) * div_coeff + (V[p - largeur] + V[p - 1] + V[p + 1] + V[p + largeur]) * div_coeff2;
               /*	 if(j==50 && i==50) printf("%f %f %f %f , ",M_u,M_v,U[p],V[p]); */

               /*Calcul des vecteurs vitesse*/
               v_Ix = Ix[p];
               v_Iy = Iy[p];
               U[p] = M_u - ((v_Ix * (v_Ix * M_u + v_Iy * M_v + Idt[p])) / (alpha + v_Ix * v_Ix + v_Iy * v_Iy));
               V[p] = M_v - ((v_Iy * (v_Ix * M_u + v_Iy * M_v + Idt[p])) / (alpha + v_Ix * v_Ix + v_Iy * v_Iy));
            }
         }
         /*******************************************************/
      }
   }
   else
   {

      for (j = hauteur - 2; j--;)
      {
         //dprints("-No-");
         q = (j + 1) * largeur + 1;
         for (i = largeur - 2; i--;) /* for (i=1; i<largeur-1; i++) */
         {
            p = i + q;

            /*Calcul des vecteurs vitesse*/

            U[p] = 0; // check the values
            V[p] = 0;
         }
      }
   }
   /*dprints("flow result \n"); */
   /*mvt_uv_to_hsl(largeur, hauteur, U, V, iv);*/

   memcpy(It1, It, largeur * hauteur * sizeof(unsigned char)); /* sauvegarde de l'image d'entree  pour creer z-1 */

   /*free(image_fond->images_table[0]) ;
    free(image_pers->images_table[0]) ;

    printf("sauvegarde du resultat dans image_soustraction.png.\n") ; */
   /*   save_png_to_disk("image_soustraction.png", *image_res, compression) ; */

   /* premiere fonction pour tests */

}

void function_optical_flow_2_images(int Gpe)
   {
      int l;
      int i, j, p, q;
      int iter;
      int largeur = 0, hauteur = 0;
      int Gpe_it = -1, Gpe_it1 = -1;
      int nb_iterations = 1;
      float alpha = 1.;
      unsigned char *It = NULL, *It1 = NULL;
      float M_u, M_v;
      float *U = NULL, *V = NULL, *iv = NULL;
      float *Ix = NULL, *Iy = NULL, *Idt = NULL;

      prom_images_struct *prom_It = NULL, *prom_It1 = NULL;
      prom_images_struct *prom_U = NULL, *prom_V = NULL, *prom_iv = NULL;
      prom_images_struct *prom_Ix = NULL, *prom_Iy = NULL, *prom_Idt = NULL;

      MyData_f_optical_flow *my_data;

      float pt_c, pt_d, pt_b, pt_bd;
      float pt1_c, pt1_d, pt1_b, pt1_bd;
      float v_Ix, v_Iy;

      char resultat[256];

      /* Recherche des deux gpes d'entreee  */
      if (def_groupe[Gpe].data == NULL)
      {
         i = 0;
         l = find_input_link(Gpe, i);
         while (l != -1)
         {
            printf("lien %d: %s--\n", i, liaison[l].nom);
            if (prom_getopt(liaison[l].nom, "-I", resultat) == 1)
            {
               Gpe_it = liaison[l].depart;
               prom_It = (prom_images_struct *) def_groupe[Gpe_it].ext;
            }
            if (prom_getopt(liaison[l].nom, "-I1", resultat) == 1)
            {
               Gpe_it1 = liaison[l].depart;
               prom_It1 = (prom_images_struct *) def_groupe[Gpe_it1].ext;
            }
            else if (prom_getopt(liaison[l].nom, "-N", resultat) == 2)
            {
               nb_iterations = atoi(resultat);
            }
            if (prom_getopt(liaison[l].nom, "-alpha", resultat) == 2)
            {
               alpha = atof(resultat);
            }

            i++;
            l = find_input_link(Gpe, i);
         }
         if (Gpe_it == -1 || Gpe_it1 == -1)
         {
            printf("manque un groupe dans f_optical_flow %d %d \n", Gpe_it, Gpe_it1);
            exit(0);
         }

         my_data = (MyData_f_optical_flow *) malloc(sizeof(MyData_f_optical_flow));
         if (my_data == NULL)
         {
            EXIT_ON_ERROR("erreur malloc dans f_optical_flow\n");
         }
         /* Test pour voir si les images sont vides */
         if (prom_It == NULL)
         {
            EXIT_ON_ERROR("Probleme (f_optical_flow) : il n'y pas d'image dans le groupe %i \n", Gpe_it);
         }

         if (prom_It1 == NULL)
         {
            EXIT_ON_ERROR("Probleme (f_optical_flo) : il n'y pas d'image dans le groupe %i\n", Gpe_it1);
         }
         /* Pour verifier que les deux images ont bien la meme taille */
         if ((prom_It->sx != prom_It1->sx) || (prom_It->sy != prom_It1->sy))
         {
            EXIT_ON_ERROR("ATTENTION!\nLes deux images en entree doivent avoir la meme taille.\nImpossible de les soustraire l'une a l'autre.\n");
         }
         else
         {
            largeur = prom_It->sx;
            hauteur = prom_It->sy;
         }
         /* Allocation memoire pour l'image resultat */
         if (def_groupe[Gpe].ext == NULL)
         {
            prom_iv = calloc_prom_image(1, largeur, hauteur, 4);
            prom_U = calloc_prom_image(1, largeur, hauteur, 4);
            prom_V = calloc_prom_image(1, largeur, hauteur, 4);
            prom_Ix = calloc_prom_image(1, largeur, hauteur, 4);
            prom_Iy = calloc_prom_image(1, largeur, hauteur, 4);
            prom_Idt = calloc_prom_image(1, largeur, hauteur, 4);
            def_groupe[Gpe].ext = prom_iv;
         }

         my_data->Gpe_it = Gpe_it;
         my_data->Gpe_it1 = Gpe_it1;
         my_data->alpha = alpha;
         my_data->nb_iterations = nb_iterations;
         my_data->prom_It = prom_It;
         my_data->prom_It1 = prom_It1;

         It = my_data->It = prom_It->images_table[0];
         It1 = my_data->It1 = prom_It1->images_table[0];

         iv = my_data->iv = (float *) prom_iv->images_table[0];

         U = my_data->U = (float *) prom_U->images_table[0];
         V = my_data->V = (float *) prom_V->images_table[0];
         Ix = my_data->Ix = (float *) prom_Ix->images_table[0];
         Iy = my_data->Iy = (float *) prom_Iy->images_table[0];
         Idt = my_data->Idt = (float *) prom_Idt->images_table[0];

         def_groupe[Gpe].data = (MyData_f_optical_flow *) my_data;
      }
      else
      {
         my_data = (MyData_f_optical_flow *) (def_groupe[Gpe].data);
         Gpe_it = my_data->Gpe_it;
         Gpe_it1 = my_data->Gpe_it1;
         It = my_data->It;
         It1 = my_data->It1;

         U = my_data->U;
         V = my_data->V;
         Ix = my_data->Ix;
         Iy = my_data->Iy;
         Idt = my_data->Idt;

         prom_It = my_data->prom_It;
         prom_It1 = my_data->prom_It1;
         prom_U = my_data->prom_U;
         prom_V = my_data->prom_V;
         prom_Ix = my_data->prom_Ix;
         prom_Iy = my_data->prom_Iy;
         prom_Idt = my_data->prom_Idt;

         alpha = my_data->alpha;
         nb_iterations = my_data->nb_iterations;

         iv = my_data->iv;
         largeur = prom_It->sx;
         hauteur = prom_It->sy;
      }

      dprints("optical_flow nb_iterations= %d alpha=%f (%d %d)\n", nb_iterations, alpha, largeur, hauteur);

      /* Calcul des Dérivées en X,Y et T */

      for (j = hauteur - 2; j--;)
      {
         /*	printf("j=%d : \n",j);*/
         q = (j + 1) * largeur + 1;
         for (i = largeur - 2; i--;)
         {
            /*	printf("i=%d ",i);*/
            p = i + q;
            pt_c = It[p]; /* Pixel central dans l'image au temps t         */
            pt_d = It[p + 1]; /* Pixel à droite dans l'image au temps t        */
            pt_b = It[p + largeur]; /* Pixel en bas dans l'image au temps t          */
            pt_bd = It[p + 1 + largeur]; /* Pixel en bas à droite dans l'image au temps t */

            pt1_c = It1[p]; /* Pixel central dans l'image au temps t         */
            pt1_d = It1[p + 1]; /* Pixel à droite dans l'image au temps t        */
            pt1_b = It1[p + largeur]; /* Pixel en bas dans l'image au temps t          */
            pt1_bd = It1[p + 1 + largeur]; /* Pixel en bas à droite dans l'image au temps t */

            /*Calcul de la dérivée en Y */
            Iy[p] = ((pt_bd + pt_b - pt_c - pt_d) + (pt1_bd + pt1_b - pt1_c - pt1_d)) / 4;
            /****************************/

            /*Calcul de la dérivée en X */
            Ix[p] = ((pt_d + pt_bd - pt_c - pt_b) + (pt1_d + pt1_bd - pt1_c - pt1_b)) / 4;

            /****************************/
            /*Calcul de la dérivée en T */
            Idt[p] = ((pt1_c + pt1_b + pt1_d + pt1_bd) - (pt_c + pt_b + pt_d + pt_bd)) / 4.;
            /*	 if(j%5==0 && i%5==0) printf("%f %f %f,",Iy[p],Ix[p],Idt[p]);*/
            /****************************/
         }
      }
      /***********************************/

      /* DEBUT DES ITERATIONS */

      for (iter = nb_iterations; iter--;) /* on s'arrete a 0 */
      {
         /*  printf("Iteration numéro  : %d \n",iter); */

         /***********Boucle de parcours des images*************/
         /*ATTENTION  : les première et dernières lignes et colonnes ne sont pas traitée */

         /*Copie des matrices tempons pour la prochaine itération */
         /*copy_matrix(M_u,nrl,nrh,ncl,nch,Tmp_u,nrl,nrh,ncl,nch);*/
         /*copy_matrix(M_v,nrl,nrh,ncl,nch,Tmp_v,nrl,nrh,ncl,nch);*/
         /*************************************************************************/

         for (j = hauteur - 2; j--;)
         {
            q = (j + 1) * largeur + 1;
            for (i = largeur - 2; i--;) /* for (i=1; i<largeur-1; i++) */
            {
               p = i + q;
               /*Calcul de la moyenne des vecteurs en X */
               /* note PG: M_u et M_v ont ils besoin d'etre sauvegarde dans des vecteurs??? */
               M_u = (U[p - largeur - 1] + U[p - largeur + 1] + U[p + largeur - 1] + U[p + largeur + 1]) / 12. + (U[p - largeur] + U[p - 1] + U[p + 1] + U[p + largeur]) / 6.;

               /*Calcul de la moyenne des vecteurs en Y */
               M_v = (V[p - largeur - 1] + V[p - largeur + 1] + V[p + largeur - 1] + V[p + largeur + 1]) / 12. + (V[p - largeur] + V[p - 1] + V[p + 1] + V[p + largeur]) / 6.;

               /*Calcul des vecteurs vitesse*/
               v_Ix = Ix[p];
               v_Iy = Iy[p];
               U[p] = M_u - ((v_Ix * (v_Ix * M_u + v_Iy * M_v + Idt[p])) / (alpha + v_Ix * v_Ix + v_Iy * v_Iy));
               V[p] = M_v - ((v_Iy * (v_Ix * M_u + v_Iy * M_v + Idt[p])) / (alpha + v_Ix * v_Ix + v_Iy * v_Iy));
            }
         }
         /*******************************************************/

      }
      dprints("flow result \n");
      for (j = hauteur; j--;)
      {
         q = j * largeur;
         for (i = largeur; i--;)
         {
            p = i + q;
            iv[p] = sqrt(U[p] * U[p] + V[p] * V[p]);
            /*	if(iv[p]>0.) printf("%d %d = %f \n",i,j,iv[p]); */
         }
      }
   }
