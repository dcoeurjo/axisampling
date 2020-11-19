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


svg::Point rot(const svg::Point &p, double theta, const svg::Point&center)
{
  return svg::Point( center.x+(p.x - center.x)*cos(theta) - (p.y-center.y)*sin(theta) ,
                    center.y+(p.x - center.x)*sin(theta) + (p.y-center.y)*cos(theta));
}

void drawFrame(svg::Document & doc,
               unsigned int i, unsigned int j,
               double density, unsigned int size,
               unsigned int width, unsigned int height)
{
  double factor = (i*i/((double)width*width)+j*j/((double)height*height));
  double theta=M_PI_4*factor;
  svg::Point center(i,j);
  
  svg::Fill fill(svg::Color::Black);
  
  std::vector<Point> samples;
  ldbnBNOT(density, samples);
  //std::cout<<"Samples count= "<<samples.size()<<std::endl;
  double dx = U(generator);
  double dy = U(generator);
  for(auto s:samples)
  {
    svg::Point sample(svg::Point(center.x + fmod(dx + s[0],1)*size, center.y + fmod(dy + s[1],1.0)*size) );
    doc << svg::Circle(rot(sample,theta,center), size/15.0, fill, svg::Stroke(1, svg::Color::Black));
  }
  
  svg::Polygon frame(svg::Stroke(1, svg::Color::Blue));
  frame << center<< rot(svg::Point(i+size, j), theta,center)
        << rot(svg::Point(i+size, j+size), theta,center) << rot(svg::Point(i,j+size), theta,center);
  doc << frame;
}


int main(int argc, char** argv)
{
  CLI::App app{"localDensit"};
  std::string inputFilename;
  app.add_option("-i,--input", inputFilename, "Input image filename")->required()->check(CLI::ExistingFile);;
  std::string outputFilename= "output.svg";
  app.add_option("-o,--output", outputFilename, "Output SVG filename")->required();
  unsigned int gridSize=10;
  app.add_option("-g,--grid", gridSize, "Grid size")->required();
  bool silent = false;
  app.add_flag("--silent", silent, "No verbose messages");
  CLI11_PARSE(app, argc, argv);
  
  //Image loading
  int width,height, nbChannels;
  unsigned char *source = stbi_load(inputFilename.c_str(), &width, &height, &nbChannels, 0);
  if (!silent) std::cout<< "Source image: "<<width<<"x"<<height<<"   ("<<nbChannels<<")"<< std::endl;
  
  //SVG init
  svg::Dimensions dimensions(width, height);
  svg::Document doc(outputFilename, svg::Layout(dimensions, svg::Layout::BottomLeft));


  initSamplers();

 
  auto getDensity=[&](unsigned int i, unsigned int j){
    double val = 0;
    for(auto y=j; y < std::min((unsigned int)height,j+gridSize);++y)
     for(auto x=i; x < std::min((unsigned int)width,i+gridSize); ++x)
    {
      auto indexPixel = nbChannels*(width*y+x);
      unsigned char r = source[ indexPixel ];
      unsigned char g = source[ indexPixel + 1];
      unsigned char b = source[ indexPixel + 2];
      val += (r+g+b)/3.0;
     
    }
    val /= (double)(gridSize*gridSize);
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
      drawFrame(doc, i, j, (getDensity(i,j)-mmin)/(mmax-mmin)*100.0, gridSize, width, height);
  
  doc.save();
  
  return 0;
}

