#pragma once

#include "ui/gl/Types.h"

namespace Game3 {
	class HasAlignment {
		public:
			HasAlignment(Alignment horizontal, Alignment vertical);
			HasAlignment();

			virtual Alignment getHorizontalAlignment() const;
			virtual Alignment getVerticalAlignment() const;
			virtual void setHorizontalAlignment(Alignment);
			virtual void setVerticalAlignment(Alignment);
			virtual void setAlignment(Alignment horizontal, Alignment vertical);

		protected:
			Alignment horizontalAlignment = Alignment::Start;
			Alignment verticalAlignment = Alignment::Start;
	};
}
