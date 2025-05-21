#pragma once

#include "ui/gl/Types.h"

namespace Game3 {
	class HasExpand {
		public:
			HasExpand(Expansion horizontal, Expansion vertical);
			HasExpand();

			virtual Expansion getHorizontalExpand() const;
			virtual Expansion getVerticalExpand() const;
			virtual Expansion getExpand(Orientation) const;
			virtual void setHorizontalExpand(Expansion);
			virtual void setVerticalExpand(Expansion);
			virtual void setExpand(Expansion horizontal, Expansion vertical);
			void setHorizontalExpand(bool);
			void setVerticalExpand(bool);
			void setExpand(bool horizontal, bool vertical);

		protected:
			Expansion horizontalExpand = Expansion::Shrink;
			Expansion verticalExpand = Expansion::Shrink;
	};
}
