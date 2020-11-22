/*
 Copyright (c)
 David Coeurjolly <david.coeurjolly@liris.cnrs.fr>
 
 All rights reserved.
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
 
 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.
 
 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIEDi
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include <iostream>
#include <string>
#include <random>
#include "CLI11.hpp"
#include "lutLDBN.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "simple_svg_1.0.0.hpp"

std::default_random_engine generator;
std::uniform_real_distribution<double> U(0.0,1.0);

//Rotation of a point
svg::Point rot(const svg::Point &p, double theta, const svg::Point&center)
{
  return svg::Point( center.x+(p.x - center.x)*cos(theta) - (p.y-center.y)*sin(theta) ,
                    center.y+(p.x - center.x)*sin(theta) + (p.y-center.y)*cos(theta));
}

//Draw of a single frame
void drawFrame(svg::Document & doc,
               double i, double j,
               double density,
               double theta, unsigned int size,
               unsigned int width, unsigned int height,
               double circleSize)
{
  svg::Point center(i,j);
  
  svg::Fill fill;//(svg::Color::Black);
  
  //Bluenoise LD sampling
  double sampleCount = std::round(sqrt(density));
  std::vector<Point> samples;
  ldbnBNOT(sampleCount*sampleCount, samples);
  
  //Need that for densities not a square of an integer
  //(simple random picking of the first "density" samples)
  std::shuffle(samples.begin(),samples.end(),generator);
  
  //Cranley Patterson rotation between frames
  double dx = U(generator);
  double dy = U(generator);
  for(size_t i=0; i < std::min(density,(double)samples.size()); ++i)
  {
    svg::Point sample(svg::Point(center.x + fmod(dx + samples[i][0],1)*size, center.y + fmod(dy + samples[i][1],1.0)*size) );
    doc << svg::Circle(rot(sample,theta,center), circleSize, fill, svg::Stroke(1, svg::Color::Black));
  }
  
  svg::Polygon frame(svg::Stroke(1, svg::Color::Black));
  frame << center<< rot(svg::Point(i+size, j), theta,center)
        << rot(svg::Point(i+size, j+size), theta,center) << rot(svg::Point(i,j+size), theta,center);
  doc << frame;
}


int main(int argc, char** argv)
{
  CLI::App app{"localDensit"};
  std::string inputFilename;
  app.add_option("-i,--input", inputFilename, "Input image filename (RGB/RGBA)")->required()->check(CLI::ExistingFile);;
  std::string outputFilename= "output.svg";
  app.add_option("-o,--output", outputFilename, "Output SVG filename")->required();
  unsigned int gridSize=10;
  app.add_option("-g,--grid", gridSize, "Grid size")->required();
  double scale=10.0;
  app.add_option("-s,--scale", scale, "Scale");
  double coef=10.0;
  app.add_option("-c,--coef", coef, "Rotation factor");
  double circleSize=10.0;
  app.add_option("--circleSize", circleSize, "Circle size");
  double powX=6.0;
  app.add_option("--powX", powX, "Power in X");
  double powY=4.0;
  app.add_option("--powY", powY, "Power in Y");
  bool silent = false;
  app.add_flag("--silent", silent, "No verbose messages");
  CLI11_PARSE(app, argc, argv);
  
  //Image loading
  int width,height, nbChannels;
  unsigned char *source = stbi_load(inputFilename.c_str(), &width, &height, &nbChannels, 0);
  if (!silent) std::cout<< "Source image: "<<width<<"x"<<height<<"   ("<<nbChannels<<")"<< std::endl;
  
  //SVG init
  svg::Dimensions dimensions(width+5*gridSize, 2.0*height+5*gridSize);
  svg::Document doc(outputFilename, svg::Layout(dimensions, svg::Layout::TopLeft));


  initSamplers();

  auto getDensity=[&](unsigned int i, unsigned int j){
    double val = 0;
    auto cpt=0;
    for(auto y=j; y < std::min((unsigned int)height,j+gridSize);++y)
     for(auto x=i; x < std::min((unsigned int)width,i+gridSize); ++x)
    {
      cpt++;
      auto indexPixel = nbChannels*(width*y+x);
      unsigned char r = source[ indexPixel ];
      unsigned char g = source[ indexPixel + 1];
      unsigned char b = source[ indexPixel + 2];
      val += (r+g+b)/3.0;
     
    }
    val /= (double)(cpt);
    return val;
  };
  
  double mmax=0.0;
  double mmin=255.0;
  for(auto j=0u; j < height ; j+=gridSize )
   for(auto i=0u; i < width ; i+=gridSize )
  {
    auto val= getDensity(i,j);
    mmin=std::min(mmin,val);
    mmax=std::max(mmax,val);
  }
  
  for(auto j=0u; j < height ; j+=gridSize )
    for(auto i=0u; i < width ; i+=gridSize )
  {
    double strengthj =std::pow((double)j/(double)height,powY);
    double strengthi =std::pow((double)i/(double)width  ,powX);
    double dx = i*strengthi*strengthj;
    double dy = strengthi*strengthj*gridSize*scale;
      
    double x=i + dx;
    double y=j + dy;
    double theta =strengthi*strengthj*coef*M_PI;
   
    drawFrame(doc, x,y, (getDensity(i,j)-mmin)/(mmax-mmin)*100.0, theta, gridSize, width, height, circleSize);
  }
  doc.save();
  
  return 0;
}

