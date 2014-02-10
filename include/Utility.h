/*
 * Utility.h
 *
 *  Created on: 10 jul 2011
 *      Author: johan
 */

#ifndef UTILITY_H_
#define UTILITY_H_

/*
 * Utility methods reused at various places
 * TODO: Move all helper methods into this file
 */

#include <Definition.h>
#include <opengl.h>

#include <model/Object.h>

#include <iostream>
#include <vector>
#include <iterator>
#include <cstdarg>
#include <cstdio>
#include <ctime>

#include <maya/MStatus.h>
#include <maya/MObjectArray.h>
#include <maya/MPlug.h>
#include <maya/MDagPath.h>
#include <maya/MVector.h>
#include <maya/MTypeId.h>

#include <maya/MArgList.h>
#include <maya/MArgDatabase.h>
#include <maya/MSelectionList.h>

/*
 * Convenient macro for the SetupOpenGLShaders below as our shader code is often as #defines
 */

#define SETUPOPENGLSHADERS(vertex_shader_source, fragment_shader_source, uniform_names, uniform_locations, uniform_count, attrib_names, attrib_locations, attrib_count, program, vertex_shader, fragment_shader, status)	\
	{																																																						\
		static const char *vss[] = { vertex_shader_source, NULL }, *fss[] = { fragment_shader_source, NULL }, *un[] = { uniform_names, NULL }, *an[] = { attrib_names, NULL };												\
		status = SetupOpenGLShaders(vss, fss, un, uniform_locations, uniform_count, an, attrib_locations, attrib_count, program, vertex_shader, fragment_shader);															\
	}

namespace Helix {
	/*
	 * Legacy API: The following methods are part of the old structure and should be removed
	 * The reason they are still around is because of the JSON importer.
	 * some of the methods are used by the new API to save code space
	 *
	 */

	MStatus SelectedBases(MObjectArray & result);
	//MStatus SelectedStrands(MObjectArray & strands, MObjectArray & bases);
	
	bool HelixBase_NextBase(const MObject & base, const MObject & attribute, MDagPath & result, MStatus *retStatus = NULL);

	enum {
		FivePrime = 1,
		ThreePrime = 2,
		Neither = 0
	};

	//unsigned int HelixBase_endType(MObject & base, MStatus *retStatus = NULL);

	//MStatus Helix_Relatives(const MObject & helix, MObjectArray & result);

	//bool HelixBase_isForward(const MObject & base, MStatus *retStatus = NULL);

	MStatus HelixBases_sort(MObjectArray & input, MObjectArray & result);

	MStatus HelixBase_RemoveAllAimConstraints(MObject & helixBase, const char *type = "aimConstraint");

	/*
	 * These are methods introduced with the new STL and MVC oriented API
	 *
	 */

	/*
	 * Returns all selected objects that are of the given type
	 */

	MStatus GetSelectedObjectsOfType(MObjectArray & objects, MTypeId & type);

	/*
	 * If object is of type `type` it is returned. If it's not, all its parents are searched
	 */

	MObject GetObjectOrParentsOfType(MObject & object, MTypeId & type, MStatus & status);

	/*
	 * These are some templates extending the algorithm methods of the STL.
	 * They should not be used where the standard STL methods can be used instead
	 */

	/*
	 * The std::for_each passes by argument, but i need to pass as reference
	 */

	template<typename It, typename Functor>
	void for_each_ref(It it, It end, Functor & func) {
		for(; it != end; ++it)
			func(*it);
	}

	/*
	 * FIXME: Try to use the std::for_each on all, even though in this case we really need the iterator to be passed as reference
	 */

	template<typename It, typename Functor>
	void for_each_itref(It & it, It end, Functor func) {
		for(; it != end; ++it)
			func(*it);
	}

	template<typename It, typename Functor>
	void for_each_ref_itref(It & it, It end, Functor & func) {
		for(; it != end; ++it)
			func(*it);
	}

	/*
	 * The std::find passes by const, which is not optimal as the Object class does some caching
	 */

	template<typename It, typename ElementT>
	It find_nonconst(It it, It end, ElementT & element) {
		for(; it != end; ++it) {
			if (*it == element)
				return it;
		}

		return end;
	}

	template<typename It, typename ElementT>
	It find_itref_nonconst(It & it, It end, ElementT & element) {
		for(; it != end; ++it) {
			if (*it == element)
				return it;
		}

		return end;
	}

	template<typename It, typename FunctT>
	It find_if_itref_nonconst(It & it, It end, FunctT funct) {
		for(; it != end; ++it) {
			if (funct(*it))
				return it;
		}

		return end;
	}

	/*
	 * This method is for solving a bug in opening Maya files containing helices. FIXME: Move function definition and declaration
	 */

	void MSceneMessage_AfterImportOpen_CallbackFunc(void *callbackData);

	/*
	 * Missing useful math methods.
	 */

	template<typename T>
	int sgn(T val) {
		return (T(0) < val) - (val < T(0));
	}

	template<typename T>
	int sgn_nozero(T val) {
		return (T(0) <= val) - (val < T(0));
	}

	template<typename T>
	T toRadians(T degrees) {
		return T(degrees * M_PI / 180);
	}

	template<typename T>
	T toDegrees(T radians) {
		return T(radians * 180 / M_PI);
	}

	/*
	 * In comparison to MVector::angle this also considers the sign when doing a rotation
	 * along the given normal axis.
	 */
	double signedAngle(const MVector & from, const MVector & to, const MVector & normal);

	/*
	 * For commands: find all the nodes identified by the names given as arguments to parameters
	 * The original version used member function pointers, thus:
	 * MStatus (MSelectionList::* GetElementFunc) (unsigned int index, ElementT & element) const
	 * worked, but only for getDependNode, as getDagPath has a third argument with a default value
	 */

	template<typename ArrayT, typename ElementT, ElementT (*GetElementFunc) (const MSelectionList & list, unsigned int index)>
	MStatus ArgList_ArgumentNodes(const MArgList & args, const MSyntax & syntax, const char *flag, ArrayT & array) {
		MStatus status;
		MArgDatabase argDatabase(syntax, args, &status);

		if (!status) {
			status.perror("MArgDatabase::#ctor");
			return status;
		}

		bool isFlagSet = argDatabase.isFlagSet(flag, &status);

		if (!status) {
			status.perror("MArgDatabase::isFlagSet");
			return status;
		}

		if (!isFlagSet)
			return MStatus::kNotFound;

		MSelectionList list;

		unsigned int num = argDatabase.numberOfFlagUses(flag);

		for(unsigned int i = 0; i < num; ++i) {
			MString path;

			if (!(status = argDatabase.getFlagArgument(flag, i, path))) {
				status.perror("MArgDatabase::getFlagArgument");
				return status;
			}

			if (!(status = list.add(path))) {
				status.perror(MString("MSelectionList::add: The object \"") + path + "\" was not found");
				//return status;
			}
		}

		/*
		 * Now extract the dagpaths or objects from the MSelectionList
		 */

		for(unsigned int i = 0; i < list.length(); ++i) {
			ElementT element;

			/*
			if (!(status = (list.*GetElementFunc) (i, element))) {
				status.perror("MSelectionList::*GetElementFunc");
				return status;
			}
			*/

			array.append(GetElementFunc(list, i));
		}

		return MStatus::kSuccess;
	}

	/*
	 * The above method was written to not distinguish between MDagPaths and MObjects, but as its syntax is a bit messy
	 */

	MStatus ArgList_GetObjects(const MArgList & args, const MSyntax & syntax, const char *flag, MObjectArray & result);
	MStatus ArgList_GetDagPaths(const MArgList & args, const MSyntax & syntax, const char *flag, MDagPathArray & result);

	/*
	 * We could work with the lists of helices/bases directly and save some performance
	 */

	template<typename ElementT, typename ArrayT, typename ContainerT, MStatus (*GetElementsFunc) (const MArgList &, const MSyntax &, const char *, ArrayT &) >
	MStatus ArgList_GetModelObjectsT(const MArgList & args, const MSyntax & syntax, const char *flag, ContainerT & result) {
		MStatus status;
		ArrayT objects;

		if (!(status = GetElementsFunc(args, syntax, flag, objects))) {
			status.perror("GetElementsFunc. This error is expected if no -b arguments were given.");
			return status;
		}

		std::copy(&objects[0], &objects[0] + objects.length(), std::back_insert_iterator<ContainerT> (result));

		return MStatus::kSuccess;
	}

	/*
	 * Some specializations as the syntax is messy for the function above
	 */

	template<typename ContainerT>
	inline MStatus ArgList_GetModelObjects(const MArgList & args, const MSyntax & syntax, const char *flag, ContainerT & result) {
		return ArgList_GetModelObjectsT<MObject, MObjectArray, ContainerT, ArgList_GetObjects> (args, syntax, flag, result);
	}

	/*
	 * Extract CV curves from an MObject
	 */

	MStatus CVCurveFromObject(const Model::Object & object, MPointArray & array);

	/*
	 * There's a lot of OpenGL shaders being used, so saving quite a lot of lines of code using this utility method
	 * it's pretty static, but the shaders used are so primitive anyway
	 * creates a set of vertex and fragment shaders + program, compiles, attaches and links them and finds the uniforms and attributes requested
	 */

	MStatus SetupOpenGLShaders(const char **vertex_shader_source, const char **fragment_shader_source, const char **uniform_names, GLint *uniform_locations, GLsizei uniform_count, const char **attrib_names, GLint *attrib_locations, GLsizei attrib_count, GLuint & program, GLuint & vertex_shader, GLuint & fragment_shader);

	/*
	 * Returns the current date and time in a standard format. Used in certain file formats.
	 */
	std::string Date();

	/*
	 * Maya specific debug tools.
	 */

	namespace Debug {
		void printf(const char *file, const char *function, size_t line, const char *expr, ...);

		MStatus evaluate(const char *file, const char *function, size_t line, const char *expr, const MStatus & status);
	}

#define HPRINT(fmt, ...) ::Helix::Debug::printf(__FILE__, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__)

	/*
	* Execute expr and then if an error occur, print debug information.
	*/
#define HMEVALUATE(expr, status) { expr; ::Helix::Debug::evaluate(__FILE__, __FUNCTION__, __LINE__, #expr, (status)); }
	/*
	 * If status is not successful print debug information.
	 */
#define HMEVALUATE_DESCRIPTION(description, status) { ::Helix::Debug::evaluate(__FILE__, __FUNCTION__, __LINE__, description, (status)); }

	/*
	* Execute expr and then if an error occur, return from current function.
	*/
#define HMEVALUATE_RETURN(expr, status) { expr; if (!status) { ::Helix::Debug::evaluate(__FILE__, __FUNCTION__, __LINE__, #expr, status); return status; } }
	/*
	 * If status is not successful print debug information.
	 */
#define HMEVALUATE_RETURN_DESCRIPTION(description, status) if (!status) { ::Helix::Debug::evaluate(__FILE__, __FUNCTION__, __LINE__, description, (status)); return status; }
}

#endif /* UTILITY_H_ */
