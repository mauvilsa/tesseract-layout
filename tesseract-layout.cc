/**
 * Tool that does document layout analysis using tesseract
 *
 * @version $Version: 2016.10.01$
 * @author Mauricio Villegas <mauvilsa@upv.es>
 * @copyright Copyright (c) 2015-present, Mauricio Villegas <mauvilsa@upv.es>
 * @link https://github.com/mauvilsa/tesseract-layout
 * @license MIT License
 */

/*** Includes *****************************************************************/
#include <../tesseract/baseapi.h>
#include <../leptonica/allheaders.h>
#include <getopt.h>
#include <time.h>

#ifdef __TESSERACT_SOURCE__
#include "pageres.h"
#include "ocrrow.h"
#endif

/*** Definitions **************************************************************/
static char tool[] = "tesseract-layout";
static char version[] = "$Version: 2016.10.01$";

#define OUT_ASCII 0
#define OUT_XMLPAGE 1

int gb_level = 3;
int gb_format = OUT_XMLPAGE;
bool gb_regblock = true;

enum {
  OPTION_HELP       = 'h',
  OPTION_VERSION    = 'v',
  OPTION_LEVEL      = 'L',
  OPTION_FORMAT     = 'F',
  OPTION_BLOCKS     = 'B',
  OPTION_PARAGRAPHS = 'P'
};

static char gb_short_options[] = "hvL:F:FBP";

static struct option gb_long_options[] = {
    { "help",        no_argument,       NULL, OPTION_HELP },
    { "version",     no_argument,       NULL, OPTION_VERSION },
    { "level",       required_argument, NULL, OPTION_LEVEL },
    { "format",      required_argument, NULL, OPTION_FORMAT },
    { "blocks",      no_argument,       NULL, OPTION_BLOCKS },
    { "paragraphs",  no_argument,       NULL, OPTION_PARAGRAPHS },
    { 0, 0, 0, 0 }
  };

/*** Functions ****************************************************************/
#define strbool( cond ) ( ( cond ) ? "true" : "false" )

void print_usage( FILE *file ) {
  fprintf( file, "Description: Document layout analysis using tesseract\n" );
  fprintf( file, "Usage: %s [OPTIONS] IMAGE\n", tool );
  fprintf( file, "Options:\n" );
  fprintf( file, "  -L, --level LEVEL\n");
  fprintf( file, "    Layout level: 1=blocks, 2=paragraphs, 3=lines, 4=words, 5=characters (def.=%d)\n", gb_level );
  fprintf( file, "  -F, --format FORMAT\n");
  fprintf( file, "    Output format, either 'ascii' or 'xmlpage' (def.=xmlpage)\n" );
  fprintf( file, "  -B, --blocks\n");
  fprintf( file, "    Use blocks for the TextRegions (def.=%s)\n", strbool(gb_regblock) );
  fprintf( file, "  -P, --paragraphs\n");
  fprintf( file, "    Use paragraphs for the TextRegions (def.=%s)\n", strbool(!gb_regblock) );
  fprintf( file, "  -h, --help\n");
  fprintf( file, "    Print this usage information and exit\n" );
  fprintf( file, "  -v, --version\n");
  fprintf( file, "    Print version and exit\n" );
}

#ifdef __TESSERACT_SOURCE__
namespace tesseract {

  class MyPageIterator : public PageIterator {
  public:
    MyPageIterator(PAGE_RES* page_res, Tesseract* tesseract,
                   int scale, int scaled_yres,
                   int rect_left, int rect_top,
                   int rect_width, int rect_height) :
      PageIterator(page_res, tesseract,
                   scale, scaled_yres,
                   rect_left, rect_top,
                   rect_width, rect_height) {}

    float getXHeight() { return it_->row()->row->x_height(); };
  };

  class MyTessBaseAPI : public TessBaseAPI {
  public:
    MyPageIterator* MyAnalyseLayout() {
      if (FindLines() == 0) {
        if (block_list_->empty())
          return NULL;  // The page was empty.
        bool merge_similar_words = false; // Merges all words in a line as a single word
        page_res_ = new PAGE_RES(merge_similar_words, block_list_, NULL); // tesseract 3.04
        //page_res_ = new PAGE_RES(block_list_, NULL); // tesseract 3.02
        DetectParagraphs(false);
        return new MyPageIterator(
            page_res_, tesseract_, thresholder_->GetScaleFactor(),
            thresholder_->GetScaledYResolution(),
            rect_left_, rect_top_, rect_width_, rect_height_);
      }
      return NULL;
    }
  };

}
#endif

/*** Program ******************************************************************/
int main( int argc, char *argv[] ) {
  int err = 0;

  /// Parse input arguments ///
  int n,m;
  while( ( n = getopt_long(argc,argv,gb_short_options,gb_long_options,&m) ) != -1 )
    switch( n ) {
      case OPTION_LEVEL:
        gb_level = atoi(optarg);
        if( gb_level < 1 || gb_level > 5 ) {
          fprintf( stderr, "%s: error: invalid layout level: %s\n", tool, optarg );
          return 1;
        }
        break;
      case OPTION_FORMAT:
        if( ! strcasecmp(optarg,"ascii") )
          gb_format = OUT_ASCII;
        else if( ! strcasecmp(optarg,"xmlpage") )
          gb_format = OUT_XMLPAGE;
        else {
          fprintf( stderr, "%s: error: unknown output format: %s\n", tool, optarg );
          return 1;
        }
        break;
      case OPTION_BLOCKS:
        gb_regblock = true;
        break;
      case OPTION_PARAGRAPHS:
        gb_regblock = false;
        break;
      case OPTION_HELP:
        print_usage( stderr );
        return 0;
      case OPTION_VERSION:
        fprintf( stderr, "%s %.10s\n", tool, version+10 );
        fprintf( stderr, "compiled against tesseract %s, linked with %s\n", TESSERACT_VERSION_STR, tesseract::TessBaseAPI::Version() );
        return 0;
      default:
        fprintf( stderr, "%s: error: incorrect input argument: %s\n", tool, argv[optind-1] );
        return 1;
    }

  if( optind >= argc ) {
    fprintf( stderr, "%s: error: expected an image to process, see usage with --help\n", tool );
    return 1;
  }

  /// Read image ///
  Pix *image = pixRead( argv[optind] );

  /// Initialize tesseract ///
#ifdef __TESSERACT_SOURCE__
  tesseract::MyTessBaseAPI *tessApi = new tesseract::MyTessBaseAPI();
#else
  tesseract::TessBaseAPI *tessApi = new tesseract::TessBaseAPI();
#endif
  tessApi->InitForAnalysePage();
  //tessApi->SetPageSegMode( tesseract::PSM_AUTO_OSD );
  tessApi->SetPageSegMode( tesseract::PSM_AUTO_ONLY );
  tessApi->SetImage( image );

  /// Perform layout analysis ///
#ifdef __TESSERACT_SOURCE__
  tesseract::MyPageIterator *iter = tessApi->MyAnalyseLayout();
#else
  tesseract::PageIterator *iter = tessApi->AnalyseLayout();
#endif

  if ( iter->Empty( tesseract::RIL_BLOCK ) )
    return 0;

  /// Ouput result in the selected format ///
  bool xmlpage = gb_format == OUT_XMLPAGE ? true : false ;

  if ( xmlpage ) {
    char buf[80];
    time_t now = time(0);
    struct tm tstruct;
    tstruct = *localtime( &now );
    strftime( buf, sizeof(buf), "%Y-%m-%dT%X", &tstruct );

    printf( "<?xml version=\"1.0\" encoding=\"utf-8\"?>\n" );
    printf( "<PcGts xmlns=\"http://schema.primaresearch.org/PAGE/gts/pagecontent/2013-07-15\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:schemaLocation=\"http://schema.primaresearch.org/PAGE/gts/pagecontent/2013-07-15 http://schema.primaresearch.org/PAGE/gts/pagecontent/2013-07-15/pagecontent.xsd\">\n" );
    printf( "  <Metadata>\n" );
    printf( "    <Creator>%s</Creator>\n", tool );
    printf( "    <Created>%s</Created>\n", buf );
    printf( "    <LastChange>%s</LastChange>\n", buf );
    printf( "  </Metadata>\n" );
    printf( "  <Page imageFilename=\"%s\" imageWidth=\"%d\" imageHeight=\"%d\">\n", argv[optind], image->w, image->h );
  }

  int x1, y1, x2, y2;
  int left, top, right, bottom;
  tesseract::ParagraphJustification just;
  bool is_list, is_crown;
  int indent;

  int block = 0;
  while ( gb_level > 0 ) {
    block ++;
    iter->BoundingBox( tesseract::RIL_BLOCK, &left, &top, &right, &bottom );
    if ( ! xmlpage )
      printf( "block %d : %dx%d+%d+%d\n", block, right-left, bottom-top, left, top );
    else if ( gb_regblock ) {
      printf( "    <TextRegion id=\"b%d\">\n", block );
      printf( "      <Coords points=\"%d,%d %d,%d %d,%d %d,%d\"/>\n",
        left, top,   right, top,   right, bottom,   left, bottom );
    }

    int para = 0;
    while ( gb_level > 1 ) {
      para ++;
      iter->BoundingBox( tesseract::RIL_PARA, &left, &top, &right, &bottom );
      iter->ParagraphInfo( &just, &is_list, &is_crown, &indent );
      if ( ! xmlpage ) {
        printf( "paragraph %d :", para );
        if ( just == tesseract::JUSTIFICATION_LEFT )
          printf( " left" );
        else if ( just == tesseract::JUSTIFICATION_CENTER )
          printf( " center" );
        else if ( just == tesseract::JUSTIFICATION_RIGHT )
          printf( " right" );
        if ( is_list )
          printf( " list" );
        if ( is_crown )
          printf( " crown" );
        else if ( indent != 0 )
          printf( " %d", indent );
        printf( " %dx%d+%d+%d\n", right-left, bottom-top, left, top );
      }
      else if ( ! gb_regblock ) {
        printf( "    <TextRegion id=\"b%d_p%d\">\n", block, para );
        printf( "      <Coords points=\"%d,%d %d,%d %d,%d %d,%d\"/>\n",
          left, top,   right, top,   right, bottom,   left, bottom );
      }

      int line = 0;
      while ( gb_level > 2 ) {
        line ++;
        iter->BoundingBox( tesseract::RIL_TEXTLINE, &left, &top, &right, &bottom );
        iter->Baseline( tesseract::RIL_TEXTLINE, &x1, &y1, &x2, &y2 );

#ifdef __TESSERACT_SOURCE__
        double xheight = iter->getXHeight();
#endif

        if ( ! xmlpage )
#ifdef __TESSERACT_SOURCE__
          printf( "line %d : %d,%d %d,%d %dx%d+%d+%d %g\n", line, x1, y1, x2, y2, right-left, bottom-top, left, top, xheight );
#else
          printf( "line %d : %d,%d %d,%d %dx%d+%d+%d\n", line, x1, y1, x2, y2, right-left, bottom-top, left, top );
#endif
        else {
#ifdef __TESSERACT_SOURCE__
          printf( "      <TextLine id=\"b%d_p%d_l%d\" custom=\"x-height:%gpx;\">\n", block, para, line, xheight );
#else
          printf( "      <TextLine id=\"b%d_p%d_l%d\">\n", block, para, line );
#endif
          printf( "        <Coords points=\"%d,%d %d,%d %d,%d %d,%d\"/>\n",
            left, top,   right, top,   right, bottom,   left, bottom );
          printf( "        <Baseline points=\"%d,%d %d,%d\"/>\n", x1, y1, x2, y2 );
        }

        int word = 0;
        while ( gb_level > 3 ) {
          word ++;
          iter->BoundingBox( tesseract::RIL_WORD, &left, &top, &right, &bottom );
          if ( ! xmlpage )
            printf( "word %d : %dx%d+%d+%d\n", word, right-left, bottom-top, left, top );
          else {
            printf( "        <Word id=\"b%d_p%d_l%d_w%d\">\n", block, para, line, word );
            printf( "          <Coords points=\"%d,%d %d,%d %d,%d %d,%d\"/>\n",
              left, top,   right, top,   right, bottom,   left, bottom );
          }

          int glyph = 0;
          while ( gb_level > 4 ) {
            glyph ++;
            iter->BoundingBox( tesseract::RIL_SYMBOL, &left, &top, &right, &bottom );
            if ( ! xmlpage )
              printf( "glyph %d : %dx%d+%d+%d\n", glyph, right-left, bottom-top, left, top );
            else {
              printf( "          <Glyph id=\"b%d_p%d_l%d_w%d_g%d\">\n", block, para, line, word, glyph );
              printf( "            <Coords points=\"%d,%d %d,%d %d,%d %d,%d\"/>\n",
                left, top,   right, top,   right, bottom,   left, bottom );
              printf( "          </Glyph>\n" );
            }

            if ( iter->IsAtFinalElement( tesseract::RIL_WORD, tesseract::RIL_SYMBOL ) )
              break;
            iter->Next( tesseract::RIL_SYMBOL );
          }

          if ( xmlpage )
            printf( "        </Word>\n" );

          if ( iter->IsAtFinalElement( tesseract::RIL_TEXTLINE, tesseract::RIL_WORD ) )
            break;
          iter->Next( tesseract::RIL_WORD );
        }

        if ( xmlpage )
          printf( "      </TextLine>\n" );

        if ( iter->IsAtFinalElement( tesseract::RIL_PARA, tesseract::RIL_TEXTLINE ) )
          break;
        iter->Next( tesseract::RIL_TEXTLINE );
      }

      if ( xmlpage && ! gb_regblock )
        printf( "    </TextRegion>\n" );

      if ( iter->IsAtFinalElement( tesseract::RIL_BLOCK, tesseract::RIL_PARA ) )
        break;
      iter->Next( tesseract::RIL_PARA );
    }

    if ( xmlpage && gb_regblock )
      printf( "    </TextRegion>\n" );

    if ( ! iter->Next( tesseract::RIL_BLOCK ) )
      break;
  }

  if ( xmlpage )
    printf( "  </Page>\n</PcGts>\n" );

  return 0;
}
