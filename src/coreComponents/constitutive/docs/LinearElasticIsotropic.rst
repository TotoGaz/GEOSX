.. _LinearElasticIsotropic:

############################################
Linear elastic isotropic solid model
############################################

Overview
=========================

This model may be used to represents a solid material with a linear elastic isotropic response to loading.
The relationship between stress and strain is typically represented by `Hooke's Law <https://en.wikipedia.org/wiki/Hooke%27s_law>`__,
which for the case of linear elasticity, may be expressed as:

.. math::
   \sigma_{ij} = \lambda \epsilon_{kk} + 2 \mu \epsilon_{ij},

where :math:`\sigma_{ij}` is the :math:`ij` component of the cauchy stress tensor,
:math:`\epsilon_{ij}` is the :math:`ij` component of the the strain tensor,
:math:`\lambda` is the Lames elastic constant,
and :math:`\mu` is the elastic shear modulus.

Hooke's Law may also be expressed using `Voigt notation <https://en.wikipedia.org/wiki/Voigt_notation>`__ for the stress and strain tensors as:

.. math::
   \tensor{\sigma} = \tensor{C} \cdot \tensor{\epsilon},

or,

.. math::
    \begin{bmatrix}
      \sigma_{11} \\
      \sigma_{22} \\
      \sigma_{33} \\
      \sigma_{23} \\
      \sigma_{13} \\
      \sigma_{12}
    \end{bmatrix}
    =
    \begin{bmatrix}
      2\mu+\lambda  &   \lambda     &   \lambda   & 0   & 0 & 0 \\
          \lambda     &  2\mu+\lambda   &   \lambda   & 0   & 0 & 0 \\
          \lambda     &    \lambda    & 2\mu+\lambda & 0  & 0 & 0 \\
          0         &       0     &       0 &\mu  & 0 & 0 \\
      0         &           0     & 0       & 0   & \mu & 0 \\
      0         &       0     & 0       & 0   & 0 & \mu
    \end{bmatrix}
    \begin{bmatrix}
      \epsilon_{11} \\
      \epsilon_{22} \\
      \epsilon_{33} \\
      2\epsilon_{23} \\
      2\epsilon_{13} \\
      2\epsilon_{12}
    \end{bmatrix}.

Variations
==========
The application of linear elasticity as presented above is typically restricted to the case of
`infinitesimal strain <https://en.wikipedia.org/wiki/Infinitesimal_strain_theory>`__.
For the case of `infinitesimal strain <https://en.wikipedia.org/wiki/Infinitesimal_strain_theory>`__, the
above relations are applied directly.
For the case of `finite strain <https://en.wikipedia.org/wiki/Finite_strain_theory>`__,
the above relations may be slightly modified to an incremental update and rotation:

.. math::
   \Delta \tensor{\sigma} &= \tensor{C} \cdot \hat{\tensor{D}},\\
   \tensor{\sigma}^{n+1} &= \hat{\tensor{R}}( \tensor{\sigma}^{n} + \Delta \tensor{\sigma} ) \hat{\tensor{R}}^T,

where :math:`\hat{\tensor{D}}` is the "incremental rate of deformation tensor" and :math:`\hat{\tensor{R}}` is the incremental rotation tensor, which are
typically calculated from the `velocity gradient <https://en.wikipedia.org/wiki/Strain-rate_tensor>`__.
This extension into finite strain constitutes a `hypo-elastic update <https://en.wikipedia.org/wiki/Hypoelastic_material>`__,
and the choice of method to calculate :math:`\hat{\tensor{D}}`, and :math:`\hat{\tensor{R}}` determines if the update is objective.
One commonly used method is the `Hughes-Winget <https://onlinelibrary.wiley.com/doi/abs/10.1002/nme.1620151210>`__ algorithm.


Parameters
=========================

The following attributes are supported:

.. include:: /coreComponents/fileIO/schema/docs/LinearElasticIsotropic.rst

Example
=========================

.. code-block:: xml

  <Constitutive>
    <LinearElasticIsotropic name="shale"
                            defaultDensity="2700"
                            defaultBulkModulus="61.9e6"
                            defaultShearModulus="28.57e6" />
  </Constitutive>
