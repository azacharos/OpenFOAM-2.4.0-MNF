/*---------------------------------------------------------------------------*\
  =========                 |
  \\      /  F ield         | OpenFOAM: The Open Source CFD Toolbox
   \\    /   O peration     |
    \\  /    A nd           | Copyright (C) 1991-2005 OpenCFD Ltd.
     \\/     M anipulation  |
-------------------------------------------------------------------------------
License
    This file is part of OpenFOAM.

    OpenFOAM is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the
    Free Software Foundation; either version 2 of the License, or (at your
    option) any later version.

    OpenFOAM is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
    for more details.

    You should have received a copy of the GNU General Public License
    along with OpenFOAM; if not, write to the Free Software Foundation,
    Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA

Class
    gaussianForce

Description

\*----------------------------------------------------------------------------*/

#include "gaussianForce.H"
#include "addToRunTimeSelectionTable.H"
#include "IFstream.H"
#include "writeTimeData.H"


namespace Foam
{

// * * * * * * * * * * * * * * Static Data Members * * * * * * * * * * * * * //

defineTypeNameAndDebug(gaussianForce, 0);

addToRunTimeSelectionTable(gravityForce, gaussianForce, dictionary);



// * * * * * * * * * * * * * Private Member Functions  * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Constructors  * * * * * * * * * * * * * * //

//- Construct from components
gaussianForce::gaussianForce
(
    Time& time,
    const dictionary& dict
)
:
    gravityForce(time, dict),
    propsDict_(dict.subDict(typeName + "Properties")),
    startPoint_(propsDict_.lookup("startPoint")),
    endPoint_(propsDict_.lookup("endPoint")),
    normalVector_((endPoint_ - startPoint_)/mag(endPoint_ - startPoint_)),
    stress_(readScalar(propsDict_.lookup("stress"))),
//     amplitude_(readScalar(propsDict_.lookup("gaussianAmplitude"))),
    density_(readScalar(propsDict_.lookup("numberDensity"))),
    constantA_(0.0),
    constantB_(0.0),
    forceDirection_(propsDict_.lookup("forceDirection"))
{
    forceDirection_ /= mag(forceDirection_);

    scalar tF = readScalar(propsDict_.lookup("thickness"));

    scalar sigma = tF/4.0;

    constantA_ = sqrt(2.0/constant::mathematical::pi)/sigma;

    constantB_ = 2.0*sigma*sigma;

    bool output = false;

    // output force distribution

    if (propsDict_.found("output"))
    {
        output = Switch(propsDict_.lookup("output"));

        if(output)
        {
            const word fieldName = propsDict_.lookup("fieldName");
            scalar nBins = readLabel(propsDict_.lookup("nBins"));
            scalar binWidth = mag(endPoint_ - startPoint_)/(nBins);
        
            scalarField forces(nBins, 0.0);
            scalarField ys(nBins, 0.0);
        
            forAll(forces, n)
            {
                scalar y = binWidth*n;
                ys[n] = y;
                forces[n] = ((constantA_*stress_)/density_)*exp(-y*y/constantB_);
            }
        
            // compute integral
            scalar forceIntegral = 0.0;
        
            for (label n = 0; n < forces.size()-1; n++)
            {
                const scalar& r1 = ys[n]; 
                const scalar& r2 = ys[n+1];
        
                const scalar& f1 = forces[n]; 
                const scalar& f2 = forces[n+1];
        
                forceIntegral += 0.5*(r2-r1)*(f1+f2);
            }
        
            Info << "forceIntegral: " << forceIntegral << endl;

            // write out force
            fileName casePath(time.path());
            
            writeTimeData
            (
                casePath,
                "gaussian_"+fieldName+"_force.xy",
                ys,
                forces
            );

            List< Pair<scalar> > forceList(forces.size());

            forAll(forceList, bin)
            {
                forceList[bin].first() = ys[bin];
                forceList[bin].second() = forces[bin];
            }

            writeTimeData
            (
                casePath,
                "gaussian_"+fieldName+"_force",
                forceList
            );
        }
    }
}



// * * * * * * * * * * * * * * * * Selectors * * * * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * Destructor  * * * * * * * * * * * * * * * //

gaussianForce::~gaussianForce()
{}


// * * * * * * * * * * * * * * * Member Functions  * * * * * * * * * * * * * //

vector gaussianForce::force(const vector& position)
{
    vector force = vector::zero;

    scalar y = (position - startPoint_) & normalVector_;

    if((y >= 0.0) && (y <= mag(endPoint_ - startPoint_)))
    {
        force = ((constantA_*stress_)/density_)*exp(-y*y/constantB_)*forceDirection_;
    }

//     scalar y = (position - startPoint_) & normalVector_;

//     return ((constantA_*stress_)/density_)*exp(-y*y/constantB_)*forceDirection_;
    

    return force;
}

vector gaussianForce::force(const scalar& time)
{
    vector force = vector::zero;
    
    return force;
}

void gaussianForce::updateForce()
{

}

void gaussianForce::write
(
    const fileName& fixedPathName,
    const fileName& timePath
)
{

}

void gaussianForce::updateProperties
(
    const dictionary& dict
)
{
    propsDict_ = dict.subDict(typeName + "Properties");

    stress_ = readScalar(propsDict_.lookup("stress"));
}



// * * * * * * * * * * * * * * * Member Operators  * * * * * * * * * * * * * //

// * * * * * * * * * * * * * * * Friend Functions  * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * Friend Operators  * * * * * * * * * * * * * //


// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * //

} // End namespace Foam

// ************************************************************************* //
