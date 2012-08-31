#include "Tree.h"

Tree::Tree(unsigned int const* const pChildrenCountPerLevel, unsigned int const levelCount,
        Matrix* const pFeatureInformationMatrix, unsigned int const targetFeatureIndex) :
        mpChildrenCountPerLevel(pChildrenCountPerLevel), mLevelCount(levelCount), mpFeatureInformationMatrix(
                pFeatureInformationMatrix), mpStartingIndexPerLevel(
                new unsigned int[mLevelCount + 1])
{
    unsigned int cumulative_element_count = 1;
    unsigned int children_per_level = 1;

    mpStartingIndexPerLevel[0] = 0;

    for (unsigned int level = 0; level < mLevelCount; ++level)
    {
        mpStartingIndexPerLevel[level + 1] = cumulative_element_count;
        children_per_level *= mpChildrenCountPerLevel[level];
        cumulative_element_count += children_per_level;
    }

    mTreeElementCount = cumulative_element_count;
    mpIndexTree = new unsigned int[cumulative_element_count];
    mpIndexTree[0] = targetFeatureIndex;
}

Tree::~Tree()
{
    delete[] mpStartingIndexPerLevel;
    delete[] mpIndexTree;
}

void const
Tree::build()
{
    for (unsigned int level = 0; level < mLevelCount; ++level)
    {
        unsigned int const parent_count = mpStartingIndexPerLevel[level + 1]
                - mpStartingIndexPerLevel[level];

#pragma omp parallel for schedule(dynamic)
        for (unsigned int parent = 0; parent < parent_count; ++parent)
            placeElements(
                    mpStartingIndexPerLevel[level + 1] + (parent * mpChildrenCountPerLevel[level]),
                    mpChildrenCountPerLevel[level], level + 1);
    }

    // Prepare output
    mPaths.reserve(mLevelCount
            * (mTreeElementCount - mpStartingIndexPerLevel[mLevelCount]));

    for (unsigned int end_element_absolute_index = mTreeElementCount - 1;
            end_element_absolute_index >= mpStartingIndexPerLevel[mLevelCount];
            --end_element_absolute_index)
    {
        unsigned int element_absolute_index = end_element_absolute_index;

        for (unsigned int level = mLevelCount; level > 0; --level)
        {
            mPaths.push_back(mpIndexTree[element_absolute_index]);
            element_absolute_index = getParentAbsoluteIndex(element_absolute_index, level);
        }
    }
}

/* inline */unsigned int const
Tree::getParentAbsoluteIndex(unsigned int const absoluteIndex, unsigned int const level) const
{
    return (absoluteIndex - mpStartingIndexPerLevel[level]) / mpChildrenCountPerLevel[level - 1]
            + mpStartingIndexPerLevel[level - 1];
}

std::vector<unsigned int> const&
Tree::getPaths() const
{
    return mPaths;
}

bool const
Tree::hasAncestorByFeatureIndex(unsigned int const absoluteIndex, unsigned int const featureIndex,
        unsigned int level) const
{
    unsigned int parent_absolute_index = absoluteIndex;

    for (unsigned int i = level; i > 0; --i)
    {
        parent_absolute_index = getParentAbsoluteIndex(parent_absolute_index, i);
        if (mpIndexTree[parent_absolute_index] == featureIndex)
            return true;
    }

    return false;
}

bool const
Tree::hasSamePath(unsigned int const absoluteIndex1, unsigned int const absoluteIndex2,
        unsigned int const level) const
{
    unsigned int parent_absolute_index = absoluteIndex1;

    for (unsigned int i = level; i > 0; --i)
    {
        parent_absolute_index = getParentAbsoluteIndex(parent_absolute_index, i);
        if (!hasAncestorByFeatureIndex(parent_absolute_index, mpIndexTree[absoluteIndex2], i))
            return false;
    }

    return true;
}

bool const
Tree::isRedundantPath(unsigned int const absoluteIndex, unsigned int const featureIndex,
        unsigned int const level) const
{
    unsigned int const upper_bound =
            (level == mLevelCount) ? mTreeElementCount : mpStartingIndexPerLevel[level + 1];

    for (unsigned int i = mpStartingIndexPerLevel[level]; i < upper_bound; ++i)
        if (hasAncestorByFeatureIndex(i, featureIndex, level)
                && hasAncestorByFeatureIndex(absoluteIndex, mpIndexTree[i], level))
            return true;

    return false;
}

void const
Tree::placeElements(unsigned int const startingIndex, unsigned int childrenCount,
        unsigned int const level)
{
    unsigned int counter = 0;
    unsigned int const feature_count = mpFeatureInformationMatrix->getRowCount();
    unsigned int* const p_candidate_feature_indices = new unsigned int[feature_count];
    unsigned int* const p_order = new unsigned int[feature_count];
    unsigned int* const p_adaptor = new unsigned int[feature_count];
    float* const p_candidate_scores = new float[feature_count];

    for (unsigned int i = 0; i < feature_count; ++i)
    {
        if (hasAncestorByFeatureIndex(startingIndex, i, level))
            continue;

        float const candidate_feature_score = std::fabs(
                mpFeatureInformationMatrix->at(i, mpIndexTree[0]));

        unsigned int ancestor_absolute_index = startingIndex;
        float ancestry_score = 0.;

        if (level > 1)
            for (unsigned int j = level; j > 0; --j)
            {
                ancestor_absolute_index = getParentAbsoluteIndex(ancestor_absolute_index, j);
                ancestry_score += std::fabs(
                        mpFeatureInformationMatrix->at(i, mpIndexTree[ancestor_absolute_index]));
            }

        p_order[counter] = counter;
        p_adaptor[counter] = counter;
        p_candidate_feature_indices[counter] = i;
        p_candidate_scores[counter] = candidate_feature_score - (ancestry_score / level);
        ++counter;
    }

    std::sort(p_order, p_order + counter, Math::IndirectComparator(p_candidate_scores, p_adaptor));

#pragma omp critical(selection)
    {
        unsigned int children_counter = 0;
        unsigned int i = counter - 1;
        while (i >= 0 && children_counter < childrenCount)
        {
            unsigned int const index = p_candidate_feature_indices[p_order[i--]];
            if (!isRedundantPath(startingIndex + children_counter, index, level))
                mpIndexTree[startingIndex + children_counter++] = index;
        }
    }

    delete[] p_order;
    delete[] p_adaptor;
    delete[] p_candidate_feature_indices;
    delete[] p_candidate_scores;
}
