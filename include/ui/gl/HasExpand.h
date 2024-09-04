#pragma once

#include "ui/gl/Types.h"

namespace Game3 {
	class HasExpand {
		public:
			HasExpand(bool horizontal, bool vertical);
			HasExpand();

			virtual bool getHorizontalExpand() const;
			virtual bool getVerticalExpand() const;
			virtual bool getExpand(Orientation) const;
			virtual void setHorizontalExpand(bool);
			virtual void setVerticalExpand(bool);
			virtual void setExpand(bool horizontal, bool vertical);

		protected:
			bool horizontalExpand = false;
			bool verticalExpand = false;
	};
}
