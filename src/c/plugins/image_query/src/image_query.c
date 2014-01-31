/*
 * This file is part of TissueStack.
 *
 * TissueStack is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * TissueStack is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with TissueStack.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "image_query.h"

void		*init(void *args) {
	  t_args_plug	*a = (t_args_plug *)args;

	  prctl(PR_SET_NAME, "Image_plug");
	  LOG_INIT(a);

	  InitializeMagick("./");

	  // free command line args
	  a->destroy(a);

	  INFO("Image Extract Plugin: Started");
	  return (NULL);
}

void			*start(void *args) {
	t_args_plug				*a;
	FILE					*socketDescriptor;
	int 					i=0;
	t_vol					*volume = NULL;
	unsigned long long int 	offset = 0;
	unsigned int 			combined_size = 0;
	char 					*image_data = NULL;
	Image 					*img = NULL;
	PixelPacket 			px;
	//unsigned char 		pixel = '\0';
	unsigned long long 		*pixel_value = NULL;
	int			 			is_raw = 0;
	int 					dim=0;
	int 					slice=0;
	int 					x=0;
	int 					y=0;
	int width = 0;
	int height = 0;

	a = (t_args_plug *)args;
	prctl(PR_SET_NAME, "TS_QUERY");

	socketDescriptor = (FILE*)a->box;

	// param reads
	while (a->commands != NULL && a->commands[i] != NULL) {
		switch (i) {
			case 0:
				volume = load_volume(a, a->commands[i]);
				break;
			case 1:
				dim = atoi(a->commands[i]);
				break;
			case 2:
				slice = atoi(a->commands[i]);
				break;
			case 3:
				y = atoi(a->commands[i]);
				break;
			case 4:
				x = atoi(a->commands[i]);
				break;
		}
		i++;
	}

	// sanity checks
	if (i != 5) {
		write_http_response(socketDescriptor,
  			  "{\"error\": {\"description\": \"Application Exception\", \"message\": \
  			  \"Query takes inputs: volume, dimension, slice, y and x!\"}}",
  			  NULL, "application/json");
		fclose(socketDescriptor);
		return NULL;
	}

	if (volume == NULL) {
		write_http_response(socketDescriptor,
  			  "{\"error\": {\"description\": \"Application Exception\", \"message\": \
  			  \"Volume not found or null!\"}}",
  			  NULL, "application/json");
		fclose(socketDescriptor);
		return NULL;
	}

	// check if is raw file
	is_raw = israw(volume->path, volume->raw_fd);
	if (is_raw <= 0) {
		write_http_response(socketDescriptor,
  			  "{\"error\": {\"description\": \"Application Exception\", \"message\": \
  			  \"Volume has to be in RAW format to be queried!\"}}",
  			  NULL, "application/json");
		fclose(socketDescriptor);
		return NULL;
	}

	// read in whole slice data
	// TODO: once dimension and orientation have been fixed in raw file, we can go for the pixel directly
    /*
	lseek(volume->raw_fd, offset + height + width, SEEK_SET);
	read(volume->raw_fd, &pixel, 1);
	INFO("Option 1: %hu", pixel);
     */
	offset = (volume->dim_offset[dim] + (unsigned long long int)((unsigned long long int)volume->slice_size[dim] * (unsigned long long int)slice));
	get_width_height(&height, &width, dim, volume);
	combined_size = width * height;

	image_data = malloc(combined_size*sizeof(*image_data));
	lseek(volume->raw_fd, offset, SEEK_SET);
	read(volume->raw_fd, image_data, combined_size);
	lseek(volume->raw_fd, 0, SEEK_SET); // return to beginning, just to be safe

	img = extractSliceData(volume, dim, image_data, width, height, socketDescriptor);
	if (img == NULL) { // something went wrong => say good bye
		if (image_data != NULL) free(image_data);
		return NULL;
	}

	// fetch pixel value and address GraphicsMagick quantum differences among systems
    px = GetOnePixel(img, x, y);
    pixel_value = malloc(3*sizeof(*pixel_value)); // RGB
    pixel_value[0] = (unsigned long long int) px.red;
    pixel_value[1] = (unsigned long long int) px.green;
    pixel_value[2] = (unsigned long long int) px.blue;
	if (QuantumDepth != 8 && img->depth == QuantumDepth) {
		pixel_value[0] = mapUnsignedValue(img->depth, 8, pixel_value[0]);
		pixel_value[1] = mapUnsignedValue(img->depth, 8, pixel_value[0]);
		pixel_value[2] = mapUnsignedValue(img->depth, 8, pixel_value[0]);
	}

	char value[100];
	sprintf(value, "{\"response\": {\"red\": %llu, \"green\": %llu, \"blue\": %llu}}", pixel_value[0], pixel_value[1], pixel_value[2]);

	// write response into socket
	write_http_response(socketDescriptor, value, "200 OK", "application/json");
	fclose(socketDescriptor);

	// clean up
	if (image_data != NULL) free(image_data);
	if (pixel_value != NULL) free(pixel_value);
	if (img != NULL) DestroyImage(img);

	return NULL;
}

void		*unload(void *args) {
    DestroyMagick();

	INFO("Image Query Plugin: Unloaded");
	return (NULL);
}

// TODO: All this conditional mumbo jumbo has to be changed. It's not good. Raw has to have a fixed dimension order and orientation!
// future TODO: the bit depth has to become a double/float !
Image * extractSliceData(t_vol * volume, int dim, char * image_data, int width, int height, FILE * socketDescriptor) {
    ExceptionInfo exception;
    Image		*img = NULL;
    Image		*tmp = NULL;
	ImageInfo	*image_info = NULL;

	// init exception
    GetExceptionInfo(&exception);

    if ((image_info = CloneImageInfo((ImageInfo *)NULL)) == NULL) {
    	dealWithException(&exception, socketDescriptor, NULL, image_info);
    	return NULL;
     }

    if (volume->original_format == MINC &&
  	  ((volume->dim_name_char[0] == 'y' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'x' && volume->dim_name_char[dim] == 'x') ||
        (volume->dim_name_char[0] == 'z' && volume->dim_name_char[1] == 'x' && volume->dim_name_char[2] == 'y' && volume->dim_name_char[dim] == 'z') ||
        (volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'y' && (volume->dim_name_char[dim] == 'z' || volume->dim_name_char[dim] == 'y')) ||
        (volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'y' && volume->dim_name_char[2] == 'z') ||
        (volume->dim_name_char[0] == 'y' && volume->dim_name_char[1] == 'x' && volume->dim_name_char[2] == 'z' && (volume->dim_name_char[dim] == 'y' || volume->dim_name_char[dim] == 'x')))) {
  	  if ((img = ConstituteImage(height, width, "I", CharPixel, image_data, &exception)) == NULL) {
      	dealWithException(&exception, socketDescriptor, NULL, image_info);
      	return NULL;
       }

        if ((volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'y' &&
        		(volume->dim_name_char[dim] == 'z' || volume->dim_name_char[dim] == 'y'))) {
  		  tmp = img;
  		  if ((img = RotateImage(img, 90, &exception)) == NULL) {
  	    	dealWithException(&exception, socketDescriptor, tmp, image_info);
  	    	return NULL;
  		  }
  		  DestroyImage(tmp);
        } else {
      	  tmp = img;
      	  if ((img = RotateImage(img, -90, &exception)) == NULL) {
   	    	dealWithException(&exception, socketDescriptor, tmp, image_info);
   	    	return NULL;
      	  }
      	  DestroyImage(tmp);
        }
      } else {
      	if ((img = ConstituteImage(width, height, "I", CharPixel, image_data, &exception)) == NULL) {
      		dealWithException(&exception, socketDescriptor, NULL, image_info);
  	    	return NULL;
        }
    }

    if ((volume->original_format != MINC) ||  (volume->original_format == MINC &&
  	  !((volume->dim_name_char[0] == 'y' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'x' && volume->dim_name_char[dim] == 'x') ||
        (volume->dim_name_char[0] == 'z' && volume->dim_name_char[1] == 'x' && volume->dim_name_char[2] == 'y' && volume->dim_name_char[dim] == 'z') ||
        (volume->dim_name_char[0] == 'y' && volume->dim_name_char[1] == 'x' && volume->dim_name_char[2] == 'z' && (volume->dim_name_char[dim] == 'y' ||
        volume->dim_name_char[dim] == 'x')) || (volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'y' && volume->dim_name_char[2] == 'z')))) {
        tmp = img;
        if ((img = FlipImage(img, &exception)) == NULL) {
  	    	dealWithException(&exception, socketDescriptor, tmp, image_info);
  	    	return NULL;
        }
        DestroyImage(tmp);
      }

   	 if ((volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'y' && volume->dim_name_char[dim] == 'z') ||
    		(volume->dim_name_char[0] == 'x' && volume->dim_name_char[1] == 'z' && volume->dim_name_char[2] == 'y' && volume->dim_name_char[dim] == 'y')) {
        tmp = img;
        if ((img = FlopImage(img, &exception)) == NULL) {
  	    	dealWithException(&exception, socketDescriptor, tmp, image_info);
  	    	return NULL;
        }
        DestroyImage(tmp);
      }

   	// for testing purposes only!
   	/*
    strcpy(img->filename, "/tmp/thishereimage.png");
    WriteImage(image_info, img);
	*/

    return img;
}

void dealWithException(ExceptionInfo *exception, FILE * socketDescriptor, Image * img, ImageInfo * image_info) {
	char error[200];

	if (exception == NULL) return;

	CatchException(exception);

	// compose error message
	sprintf(error,
			"{\"error\": {\"description\": \"Application Exception\", \"message\": \"%s\"}}",
			exception->description != NULL ? exception->description :
					(exception->reason != NULL ? exception->reason: "N/A"));

	// write out error
	write_http_response(socketDescriptor, error, NULL, "application/json");
	fclose(socketDescriptor);

	// clean up
	if (img != NULL) DestroyImage(img);
	if (image_info != NULL) DestroyImageInfo(image_info);
}