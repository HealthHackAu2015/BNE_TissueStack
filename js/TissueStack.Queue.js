TissueStack.Queue = function (canvas) {
	return {
		canvas : canvas,
		queue_handle : null,
		drawingIntervalInMillis : 100,
		requests : [],
		presentlyQueuedZoomLevelAndSlice: null,
		lowResolutionPreviewDrawn : false,
		latestDrawRequestTimestamp : 0,
		setDrawingInterval : function(value) {
			if (typeof(value) !== 'number' || value < 0) {
				throw "Interval has to be greater or equal to 0";
			}
			this.stopQueue();
			this.drawingIntervalInMillis = value;
			this.startQueue();
		},startQueue : function() {
			if (this.queue_handle) { // already active
				return;
			}
			
			this.queue_handle = setInterval(function(_this) {
				// sanity check, if we still have some requests queued
				var latestRequest =_this.requests.pop();
				if (!latestRequest) {
					_this.stopQueue();
					return;
				}
				
				// double check if we are obsolete already
				if (_this.presentlyQueuedZoomLevelAndSlice !== ('' + latestRequest.zoom_level + '_' + latestRequest.slice)) {
					_this.clearRequestQueue();
					_this.stopQueue();
					return;
				}

				_this.clearRequestQueue();
				_this.latestDrawRequestTimestamp = latestRequest.timestamp;

				_this.prepareDrawRequest(latestRequest);
				_this.drawLowResolutionPreview(_this.latestDrawRequestTimestamp);
				_this.drawRequestAfterLowResolutionPreview(latestRequest);
			}, this.drawingIntervalInMillis , this);
		},
		stopQueue : function() {
			if (!this.queue_handle) {
				return;
			}
			clearInterval(this.queue_handle);
			this.queue_handle = null;
		},
		addToQueue : function(draw_request) {
			// we have no existing queue for the zoom level and slice =>
			// this means: A) we have to create it AND B) we have to empty the queue to get rid of old requests
			if (this.presentlyQueuedZoomLevelAndSlice !== ('' + draw_request.zoom_level + '_' + draw_request.slice)) {
				this.presentlyQueuedZoomLevelAndSlice = '' + draw_request.zoom_level + '_' + draw_request.slice;
				this.requests = [];
			}
			
			// clicks are processed instantly
			if (!draw_request.move) {
				this.clearRequestQueue();
				this.latestDrawRequestTimestamp = draw_request.timestamp;
				
				this.prepareDrawRequest(draw_request);
				this.drawLowResolutionPreview(draw_request.timestamp);
				this.drawRequestAfterLowResolutionPreview(draw_request);

				return;
			}
			
			// queue pans
			this.requests.push(draw_request);
			
			// process through queue
			this.startQueue();
		},
		drawRequestAfterLowResolutionPreview : function(draw_request, timestamp) {
			var lowResBackdrop = setInterval(function(_this, draw_request, timestamp) {
				if (_this.lowResolutionPreviewDrawn) {
					if (draw_request) {
						_this.drawRequest(draw_request);
					} else {
						_this.canvas.drawMe(timestamp);
					}
					clearInterval(lowResBackdrop);
				}
			}, 50, this, draw_request, timestamp);		
 		},
 		clearRequestQueue : function() {
			this.requests = [];
 		}, drawLowResolutionPreview : function(timestamp) {
 			// this is to prevent preview fetching for the cases when the user is navigating in a view that exceeds the data extent
 			// so that they can set the crosshair outside of the extent
 			var slice = this.canvas.getDataExtent().slice;
 			if (slice < 0 || slice > this.canvas.getDataExtent().max_slices) {
 				this.lowResolutionPreviewDrawn = true;
 				return;
 			}

 			this.lowResolutionPreviewDrawn = false;

			var ctx = this.canvas.getCanvasContext();
			
			// nothing to do if we are totally outside
			if (this.canvas.upper_left_x < 0 && (this.canvas.upper_left_x + this.canvas.getDataExtent().x) <=0
					|| this.canvas.upper_left_x > 0 && this.canvas.upper_left_x > this.canvas.dim_x
					|| this.canvas.upper_left_y <=0 || (this.canvas.upper_left_y - this.canvas.getDataExtent().y) >= this.canvas.dim_y) {
				this.lowResolutionPreviewDrawn = true;
				return;
			} 
			
			var canvasX = 0;
			var imageOffsetX = 0;
			var width = this.canvas.getDataExtent().x;
			if (this.canvas.upper_left_x < 0) {
				width += this.canvas.upper_left_x;
				imageOffsetX = this.canvas.getDataExtent().x - width;
			} else {
				canvasX = this.canvas.upper_left_x;
			}
			
			if (canvasX + width > this.canvas.dim_x) {
				width = this.canvas.dim_x - canvasX;
			}

			var canvasY = 0;
			var imageOffsetY = 0;
			var height = this.canvas.getDataExtent().y;
			if (this.canvas.upper_left_y <= this.canvas.dim_y) {
				canvasY = this.canvas.dim_y - this.canvas.upper_left_y;
			} else {
				imageOffsetY = this.canvas.upper_left_y - this.canvas.dim_y;
				height = this.canvas.getDataExtent().y - imageOffsetY;
			}
			
			if (height > this.canvas.dim_y) {
				height = this.canvas.dim_y;
			}
			
			var imageTile = new Image();
			imageTile.src = 
				TissueStack.tile_directory + this.canvas.getDataExtent().data_id + "/" + this.canvas.getDataExtent().zoom_level + "/" 
				+ this.canvas.getDataExtent().plane
				+ "/" + slice + ".low.res." + this.canvas.image_format;

			(function(_this, imageOffsetX, imageOffsetY, canvasX, canvasY, width, height) {
				imageTile.onload = function() {
					if (timestamp < _this.latestDrawRequestTimestamp) {
						return;
					}
				
					if (this.width < width) {
						width = this.width;
					}

					if (this.height < height) {
						height = this.height;
					}

					ctx.drawImage(this, imageOffsetX, imageOffsetY, width, height, canvasX, canvasY, width, height);
					_this.lowResolutionPreviewDrawn = true;
				};
			})(this, imageOffsetX, imageOffsetY, canvasX, canvasY, width, height);
 		}, prepareDrawRequest : function(draw_request) {
			var thisHerePlane = this.canvas.getDataExtent().plane;
			
			if (draw_request.plane === thisHerePlane) { // this is the own canvas  we disregard
				return;
			}

			// these are the moves caused by other canvases
			if (thisHerePlane === 'x' && draw_request.plane === 'z') {
				this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(draw_request.coords.x);
				if (draw_request.move) {
					this.canvas.setUpperLeftCorner(draw_request.upperLeftCornerOrCrossCoords.y - draw_request.max_coords_of_event_triggering_plane.max_y, this.canvas.upper_left_y);
				} else {
					this.canvas.drawCoordinateCross({x: this.canvas.dim_x - draw_request.upperLeftCornerOrCrossCoords.y, y:  this.canvas.cross_y});
				}
			} else if (thisHerePlane === 'y' && draw_request.plane === 'z') {
				this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(draw_request.max_coords_of_event_triggering_plane.max_y - draw_request.coords.y);
				if (draw_request.move) {
					this.canvas.setUpperLeftCorner(draw_request.upperLeftCornerOrCrossCoords.x , this.canvas.upper_left_y);
				} else {
					this.canvas.drawCoordinateCross({x: draw_request.upperLeftCornerOrCrossCoords.x, y: this.canvas.cross_y});
				}
			} else if (thisHerePlane === 'x' && draw_request.plane === 'y') {
				this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(draw_request.coords.x);
				if (draw_request.move) {
					this.canvas.setUpperLeftCorner(this.canvas.upper_left_x, draw_request.upperLeftCornerOrCrossCoords.y);
				} else {
					this.canvas.drawCoordinateCross({x: this.canvas.cross_x, y: draw_request.upperLeftCornerOrCrossCoords.y});
				}
			} else if (thisHerePlane === 'z' && draw_request.plane === 'y') {
				this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(draw_request.max_coords_of_event_triggering_plane.max_y - draw_request.coords.y);
				if (draw_request.move) {
					this.canvas.setUpperLeftCorner(draw_request.upperLeftCornerOrCrossCoords.x , this.canvas.upper_left_y);
				} else {
					this.canvas.drawCoordinateCross({x: draw_request.upperLeftCornerOrCrossCoords.x, y:  this.canvas.cross_y});
				}
			} else if (thisHerePlane === 'y' && draw_request.plane === 'x') {
				this.canvas.getDataExtent().setSliceWithRespectToZoomLevel(draw_request.coords.x);
				if (draw_request.move) {
					this.canvas.setUpperLeftCorner(this.canvas.upper_left_x , draw_request.upperLeftCornerOrCrossCoords.y);
				} else {
					this.canvas.drawCoordinateCross({x:   this.canvas.cross_x , y: draw_request.upperLeftCornerOrCrossCoords.y});
				}
			} else if (thisHerePlane === 'z' && draw_request.plane === 'x') {
				this.canvas.getDataExtent().setSliceWithRespectToZoomLevel((draw_request.max_coords_of_event_triggering_plane.max_y - draw_request.coords.y));
				if (draw_request.move) {
					this.canvas.setUpperLeftCorner(this.canvas.upper_left_x , draw_request.max_coords_of_event_triggering_plane.max_x + draw_request.upperLeftCornerOrCrossCoords.x);
				} else {
					this.canvas.drawCoordinateCross({x: this.canvas.cross_x, y: this.canvas.dim_y - draw_request.upperLeftCornerOrCrossCoords.x});
				}
			}
		}, drawRequest : function(draw_request) {
			// redraw
			this.canvas.drawMe(draw_request.timestamp);
			
			// tidy up where we left debris
			if (this.canvas.upper_left_x > 0) { // in front of us
				this.canvas.eraseCanvasPortion(0, 0, this.canvas.upper_left_x, this.canvas.dim_y);
			}
			if (this.canvas.upper_left_x <= 0 || this.canvas.upper_left_x <= 0 + this.canvas.getDataExtent().x < + this.canvas.dim_x){ // behind us
				this.canvas.eraseCanvasPortion(
						this.canvas.upper_left_x + this.canvas.getDataExtent().x, 0,
						this.canvas.dim_x - (this.canvas.upper_left_x + this.canvas.getDataExtent().x), this.canvas.dim_y);
			}
			
			if (this.canvas.upper_left_y < 0 || (this.canvas.upper_left_y < this.canvas.dim_y && this.canvas.upper_left_y >= 0)) { // in front of us
				this.canvas.eraseCanvasPortion(0, 0, this.canvas.dim_x, (this.canvas.upper_left_y <= 0) ? this.canvas.dim_y : (this.canvas.dim_y - this.canvas.upper_left_y));
			}
			if ((this.canvas.upper_left_y - this.canvas.getDataExtent().y) >= this.canvas.dim_y || (this.canvas.upper_left_y - this.canvas.getDataExtent().y) > 0) { // behind us
				this.canvas.eraseCanvasPortion(
					0, (this.canvas.upper_left_y >= this.canvas.dim_y && this.canvas.upper_left_y - this.canvas.getDataExtent().y >= this.canvas.dim_y) ? 0 : (this.canvas.dim_y - (this.canvas.upper_left_y - this.canvas.getDataExtent().y)),
					this.canvas.dim_x, 
					(this.canvas.upper_left_y >= this.canvas.dim_y && this.canvas.upper_left_y - this.canvas.getDataExtent().y >= this.canvas.dim_y) ?
							this.canvas.dim_y : (this.canvas.dim_y - (this.canvas.upper_left_y - this.canvas.getDataExtent().y)));
			}
		}
	};
};
