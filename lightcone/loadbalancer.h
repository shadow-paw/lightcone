#ifndef LIGHTCONE_LOADBALANCER_H__
#define LIGHTCONE_LOADBALANCER_H__

#include <atomic>

namespace lightcone {
// -----------------------------------------------------------
//! Load Balancer Interface
template <typename TOKEN>
class LoadBalancer {
 public:
    //! Setup load balancer
    //! \param[in] min minimum value to return for retain()
    //! \param[in] max maximum value to return for retain()
    //! \return true on success, false on fail with no side-effect.
    virtual bool setup(int min, int max) = 0;
    //! retain a value
    //! \param[in] token Hashable token used for load balancing logic.
    //! \return identitifer for the work load
    //! \sa release
    virtual int retain(const TOKEN& token) = 0;
    //! release a value
    //! \param[in] token Hashable token used for load balancing logic.
    //! \sa retain
    virtual void release(const TOKEN& token) = 0;
};
// -----------------------------------------------------------
//! Load balancer - Round Robin
template <typename TOKEN>
class LoadBalancerRR : public LoadBalancer<TOKEN> {
 public:
    bool setup  (int min, int max)   { _min = min; _max = max; _next = min; _mod = max - min +1; return true; }
    int  retain (const TOKEN& token) { return _min + ((_next++) % _mod); }
    void release(const TOKEN& token) { }
 private:
    int _min, _max, _mod;
    std::atomic<int> _next;
};
// -----------------------------------------------------------
}  // namespace lightcone

#endif  // LIGHTCONE_LOADBALANCER_H__
