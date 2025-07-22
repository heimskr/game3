#pragma once

#include "ui/Types.h"

namespace Game3 {
	class HasAlignment {
		public:
			HasAlignment(Alignment horizontal, Alignment vertical);
			HasAlignment();

			virtual Alignment getHorizontalAlignment() const;
			virtual Alignment getVerticalAlignment() const;
			Alignment getAlignment(Orientation) const;
			virtual void setHorizontalAlignment(Alignment);
			virtual void setVerticalAlignment(Alignment);
			void setAlignment(Alignment);
			void setAlignment(Orientation, Alignment);
			virtual void setAlignment(Alignment horizontal, Alignment vertical);

		protected:
			Alignment horizontalAlignment = Alignment::Start;
			Alignment verticalAlignment = Alignment::Start;

			void adjustCoordinate(Orientation, float &coordinate, float available_size, float widget_size);
	};
}
