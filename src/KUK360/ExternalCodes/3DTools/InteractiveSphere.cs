/* 
 * The code in this file was copied from an external project and then slightly modified to suit our needs.
 * 
 * Original project: 3DTools
 * Original repository: http://3dtools.codeplex.com
 * Original repository: https://www.codeplexarchive.org/project/3DTools
 * Original license: Microsoft Limited Permissive License
 * Original license: http://www.microsoft.com/resources/sharedsource/licensingbasics/limitedpermissivelicense.mspx
 * Original license: https://web.archive.org/web/20051029093251/http://www.microsoft.com/resources/sharedsource/licensingbasics/limitedpermissivelicense.mspx
 */

//---------------------------------------------------------------------------
//
// (c) Copyright Microsoft Corporation.
// This source is subject to the Microsoft Limited Permissive License.
// See http://www.microsoft.com/resources/sharedsource/licensingbasics/limitedpermissivelicense.mspx
// All other rights reserved.
//
// This file is part of the 3D Tools for Windows Presentation Foundation
// project.  For more information, see:
// 
// http://CodePlex.com/Wiki/View.aspx?ProjectName=3DTools
//
//---------------------------------------------------------------------------

using System;
using System.Windows;
using System.Windows.Media.Media3D;

namespace KUK360.ExternalCodes._3DTools
{
    internal static class InteractiveSphere
    {
        private static Point3D GetPosition(double theta, double phi, double radius)
        {
            double x = radius * Math.Sin(theta) * Math.Sin(phi);
            double z = radius * Math.Cos(phi);
            double y = radius * Math.Cos(theta) * Math.Sin(phi);

            return new Point3D(x, y, z);
        }

        internal static Vector3D GetNormal(double theta, double phi)
        {
            return (Vector3D)GetPosition(theta, phi, 1.0);
        }

        internal static double DegToRad(double degrees)
        {
            return (degrees / 180.0) * Math.PI;
        }

        private static Point GetTextureCoordinate(double theta, double phi)
        {
            Point p = new Point(theta / (2 * Math.PI), 
                                phi  / (Math.PI));           

            return p;
        }

        internal static MeshGeometry3D Tessellate(int tDiv, int pDiv, double radius)
        {
            double dt = DegToRad(360.0) / tDiv;
            double dp = DegToRad(180.0) / pDiv;

            MeshGeometry3D mesh = new MeshGeometry3D();

            for (int pi = 0; pi <= pDiv; pi++)
            {
                double phi = pi * dp;

                for (int ti = 0; ti <= tDiv; ti++)
                {
                    // we want to start the mesh on the x axis
                    double theta = ti * dt;

                    mesh.Positions.Add(GetPosition(theta, phi, radius));
                    mesh.Normals.Add(GetNormal(theta, phi));
                    mesh.TextureCoordinates.Add(GetTextureCoordinate(theta, phi));
                }
            }

            for (int pi = 0; pi < pDiv; pi++)
            {
                for (int ti = 0; ti < tDiv; ti++)
                {
                    int x0 = ti;
                    int x1 = (ti + 1);
                    int y0 = pi * (tDiv + 1);
                    int y1 = (pi + 1) * (tDiv + 1);

                    mesh.TriangleIndices.Add(x0 + y0);
                    mesh.TriangleIndices.Add(x0 + y1);
                    mesh.TriangleIndices.Add(x1 + y0);

                    mesh.TriangleIndices.Add(x1 + y0);
                    mesh.TriangleIndices.Add(x0 + y1);
                    mesh.TriangleIndices.Add(x1 + y1);
                }
            }

            mesh.Freeze();
            return mesh;
        }
    }
}
