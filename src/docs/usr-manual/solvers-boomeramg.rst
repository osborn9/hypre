.. Copyright 1998-2019 Lawrence Livermore National Security, LLC and other
   HYPRE Project Developers. See the top-level COPYRIGHT file for details.

   SPDX-License-Identifier: (Apache-2.0 OR MIT)


BoomerAMG
==============================================================================

BoomerAMG is a parallel implementation of the algebraic multigrid method
[RuSt1987]_.  It can be used both as a solver or as a preconditioner.  The user
can choose between various different parallel coarsening techniques,
interpolation and relaxation schemes.  While the default settings work fairly
well for two-dimensional diffusion problems, for three-dimensional diffusion
problems, it is recommended to choose a lower complexity coarsening like HMIS or
PMIS (coarsening 10 or 8) and combine it with a distance-two interpolation
(interpolation 6 or 7), that is also truncated to 4 or 5 elements per
row. Additional reduction in complexity and increased scalability can often be
achieved using one or two levels of aggressive coarsening.


Parameter Options
------------------------------------------------------------------------------

Various BoomerAMG functions and options are mentioned below. However, for a
complete listing and description of all available functions, see the reference
manual.


BoomerAMG's Create function differs from the synopsis in that it has only one
parameter ``HYPRE_BoomerAMGCreate(HYPRE_Solver *solver)``. It uses the
communicator of the matrix A.


Coarsening Options
------------------------------------------------------------------------------

Coarsening can be set by the user using the function
``HYPRE_BoomerAMGSetCoarsenType``. A detailed description of various coarsening
techniques can be found in [HeYa2002]_, [Yang2005]_.

Various coarsening techniques are available:

* the Cleary-Luby-Jones-Plassman (CLJP) coarsening,
* the Falgout coarsening which is a combination of CLJP and the classical RS
  coarsening algorithm,
* CGC and CGC-E coarsenings [GrMS2006a]_, [GrMS2006b]_,
* PMIS and HMIS coarsening algorithms which lead to coarsenings with lower
  complexities [DeYH2004]_ as well as
* aggressive coarsening, which can be applied to any of the coarsening
  techniques mentioned above a nd thus achieving much lower complexities and
  lower memory use [Stue1999]_.

To use aggressive coarsening the user has to set the number of levels to which
he wants to apply aggressive coarsening (starting with the finest level) via
``HYPRE_BoomerAMGSetAggNumLevels``. Since aggressive coarsening requires long
range interpolation, multipass interpolation is always used on levels with
aggressive coarsening, unless the user specifies another long-range
interpolation suitable for aggressive coarsening.

Note that the default coarsening is HMIS [DeYH2004]_.


Interpolation Options
------------------------------------------------------------------------------

Various interpolation techniques can be set using ``HYPRE_BoomerAMGSetInterpType``:

* the "classical" interpolation as defined in [RuSt1987]_,
* direct interpolation [Stue1999]_,
* standard interpolation [Stue1999]_,
* an extended "classical" interpolation, which is a long range interpolation and
  is recommended to be used with PMIS and HMIS coarsening for harder problems
  [DFNY2008]_,
* multipass interpolation [Stue1999]_,
* two-stage interpolation [Yang2010]_,
* Jacobi interpolation [Stue1999]_,
* the "classical" interpolation modified for hyperbolic PDEs.

Jacobi interpolation is only use to improve certain interpolation operators and
can be used with ``HYPRE_BoomerAMGSetPostInterpType``.  Since some of the
interpolation operators might generate large stencils, it is often possible and
recommended to control complexity and truncate the interpolation operators using
``HYPRE_BoomerAMGSetTruncFactor`` and/or ``HYPRE_BoomerAMGSetPMaxElmts``, or
``HYPRE_BoomerAMGSetJacobiTruncTheshold`` (for Jacobi interpolation only).

Note that the default interpolation is extended+i interpolation [DFNY2008]_
truncated to 4 elements per row.


Non-Galerkin Options
------------------------------------------------------------------------------

In order to reduce communication, there is a non-Galerkin coarse grid
sparsification option available [FaSc2014]_.  This option can be used by itself
or with existing strategies to reduce communication such as aggressive
coarsening and HMIS coarsening.  To use, call
``HYPRE_BoomerAMGSetNonGalerkTol``, which gives BoomerAMG a list of level
specific non-Galerkin drop tolerances.  It is common to drop more aggressively
on coarser levels.  A common choice of drop-tolerances is :math:`[0.0, 0.01,
0.05]` where the value of 0.0 will skip the non-Galerkin process on the first
coarse level (level 1), use a drop-tolerance of 0.01 on the second coarse level
(level 2) and then use 0.05 on all subsequent coarse levels.  While still
experimental, this capability has significantly improved performance on a
variety of problems.  See the ``ij`` driver for an example usage and the
reference manual for more details.


Smoother Options
------------------------------------------------------------------------------

A good overview of parallel smoothers and their properties can be found in
[BFKY2011]_. Various of the described relaxation techniques are available:

* weighted Jacobi relaxation,
* a hybrid Gauss-Seidel / Jacobi relaxation scheme, 
* a symmetric hybrid Gauss-Seidel / Jacobi relaxation scheme, 
* l1-Gauss-Seidel or Jacobi,
* Chebyshev smoothers,
* hybrid block and Schwarz smoothers [Yang2004]_,
* ILU and approximate inverse smoothers.

Point relaxation schemes can be set using ``HYPRE_BoomerAMGSetRelaxType`` or, if
one wants to specifically set the up cycle, down cycle or the coarsest grid,
with ``HYPRE_BoomerAMGSetCycleRelaxType``. To use the more complicated
smoothers, e.g. block, Schwarz, ILU smoothers, it is necessary to use
``HYPRE_BoomerAMGSetSmoothType`` and
``HYPRE_BoomerAMGSetSmoothNumLevels``. There are further parameter choices for
the individual smoothers, which are described in the reference manual.  The
default relaxation type is l1-Gauss-Seidel, using a forward solve on the down
cycle and a backward solve on the up-cycle, to keep symmetry. Note that if
BoomerAMG is used as a preconditioner for conjugate gradient, it is necessary to
use a symmetric smoother. Other symmetric options are weighted Jacobi or hybrid
symmetric Gauss-Seidel.


AMG for systems of PDEs
------------------------------------------------------------------------------
 
If the users wants to solve systems of PDEs and can provide information on which
variables belong to which function, BoomerAMG's systems AMG version can also be
used. Functions that enable the user to access the systems AMG version are
``HYPRE_BoomerAMGSetNumFunctions``, ``HYPRE_BoomerAMGSetDofFunc`` and
``HYPRE_BoomerAMGSetNodal``.

If the user can provide the near null-space vectors, such as the rigid body
modes for linear elasticity problems, an interpolation is available that will
incorporate these vectors with ``HYPRE_BoomerAMGSetInterpVectors`` and
``HYPRE_BoomerAMGSetInterpVecVariant``. This can lead to improved convergence
and scalability [BaKY2010]_.


Special AMG Cycles
------------------------------------------------------------------------------

The default cycle is a V(1,1)-cycle, however it is possible to change the number
of sweeps of the up- and down-cycle as well as the coare grid. One can also
choose a W-cycle, however for parallel processing this is not recommended, since
it is not scalable.

BoomerAMG also provides an additive V(1,1)-cycle as well as a mult-additive
V(1,1)-cycle and a simplified versioni [VaYa2014]_. The additive variants can
only be used with weighted Jacobi or l1-Jacobi smoothing.


Miscellaneous
------------------------------------------------------------------------------

For best performance, it might be necessary to set certain parameters, which
will affect both coarsening and interpolation.  One important parameter is the
strong threshold, which can be set using the function
``HYPRE_BoomerAMGSetStrongThreshold``.  The default value is 0.25, which appears
to be a good choice for 2-dimensional problems and the low complexity coarsening
algorithms.  For 3-dimensional problems a better choice appears to be 0.5, when
using the default coarsening algorithm. However, the choice of the strength
threshold is problem dependent and therefore there could be better choices than
the two suggested ones.

