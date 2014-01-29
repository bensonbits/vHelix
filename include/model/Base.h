/*
 * Base.h
 *
 *  Created on: 9 feb 2012
 *      Author: johan
 */

#ifndef _MODEL_BASE_H_
#define _MODEL_BASE_H_

#include <model/Object.h>
#include <model/Material.h>
#include <view/BaseShape.h>

#include <DNA.h>

#include <maya/MString.h>
#include <maya/MTransformationMatrix.h>
#include <maya/MVector.h>
#include <maya/MEulerRotation.h>
#include <maya/MFnDagNode.h>
#include <maya/MNodeMessage.h>

namespace Helix {
	namespace Model {
		class Helix;
		/*
		 * Base: Defines the interface for manipulating helix bases
		 */

		class VHELIXAPI Base : public Object {
			friend void BaseModel_Shape_NodePreRemovalCallback(MObject & node,void *clientData);
		public:
			enum Type {
				BASE = 0,
				FIVE_PRIME_END = 1,
				THREE_PRIME_END = 2,
				END = 3
			};

			/*
			 * Null base is similar to the MObject::kNullObj and can be used as an invalid node.
			 */
			static Base null;

			static MStatus Create(Helix & helix, const MString & name, const MVector & translation, Base & base);

			DEFINE_DEFAULT_INHERITED_OBJECT_CONSTRUCTORS(Base)

			static MStatus AllSelected(MObjectArray & selectedBases);

			/*
			 * Handle materials (colors) of the base
			 */

			MStatus setMaterial(const Material & material);
			MStatus getMaterial(Material & material);

			/*
			 * Faster than the above
			 */

			MStatus getMaterialColor(float & r, float & g, float & b, float & a);

			/*Color getColor(MStatus & status);
			MStatus setColor(const Color & color);*/

			/*
			 * Returns one of the enum types above
			 */

			Type type(MStatus & status);

			/*
			 * Connect/disconnect
			 */

			MStatus connect_forward(Base & target, bool ignorePreviousConnections = false);
			MStatus disconnect_forward(bool ignorePerpendicular = false);
			MStatus disconnect_backward(bool ignorePerpendicular = false);
			MStatus connect_opposite(Base & target, bool ignorePreviousConnections = false);
			MStatus disconnect_opposite();

			/*
			 * Labels
			 */

			MStatus setLabel(DNA::Name label);
			MStatus getLabel(DNA::Name & label);

			/*
			 * Find the next, prev or opposite bases.
			 * If you want to iterate strands, use the Strand class as it is iterator based and works with STL
			 */

			Base forward(MStatus & status);
			Base forward();

			Base backward(MStatus & status);
			Base backward();

			Base opposite(MStatus & status);
			Base opposite();

			bool opposite_isDestination(MStatus & status);

			/*
			 * Returns the direction of the strand relative to a given axis.
			 * Uses forward or backward connections to figure out in what direction
			 * the strand is going and returns the sign of the dot product between this
			 * vector and the axis given.
			 *
			 * space: In what space is this calculated? Usually relative to the Helix, that is kTransform.
			 */
			int sign_along_axis(const MVector & axis, MSpace::Space space, MStatus & status);

			/*
			 * Helix
			 */

			Helix getParent(MStatus & status);

			inline MStatus setShapesVisibility(bool visible) {
				return Object::setShapesVisibility(visible, View::BaseShape::id);
			}

			inline MStatus toggleShapesVisibility() {
				return Object::toggleShapesVisibility(View::BaseShape::id);
			}

			inline bool isAnyShapeVisible(MStatus & status) {
				return Object::isAnyShapeVisible(View::BaseShape::id, status);
			}

		private:
			/*
			 * The shape is only created once, after this, it is instead instanced. Saves on file size and hopefully increases performance
			 */

			/*static MObject s_shape;
			static MCallbackId s_shape_NodePreRemovalCallbackId;*/
		};
	}
}

#endif /* _MODEL_BASE_H_ */
