//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Slicing.Objects
{
    public sealed class TestIntf : ITestIntf
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public AnyClass SBaseAsObject(Current current)
        {
            var sb = new SBase();
            sb.sb = "SBase.sb";
            return sb;
        }

        public SBase SBaseAsSBase(Current current)
        {
            var sb = new SBase();
            sb.sb = "SBase.sb";
            return sb;
        }

        public SBase SBSKnownDerivedAsSBase(Current current)
        {
            var sbskd = new SBSKnownDerived();
            sbskd.sb = "SBSKnownDerived.sb";
            sbskd.sbskd = "SBSKnownDerived.sbskd";
            return sbskd;
        }

        public SBSKnownDerived SBSKnownDerivedAsSBSKnownDerived(Current current)
        {
            var sbskd = new SBSKnownDerived();
            sbskd.sb = "SBSKnownDerived.sb";
            sbskd.sbskd = "SBSKnownDerived.sbskd";
            return sbskd;
        }

        public SBase SBSUnknownDerivedAsSBase(Current current)
            => new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud");

        public SBase SBSUnknownDerivedAsSBaseCompact(Current current) =>
            new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud");

        public AnyClass SUnknownAsObject(Current current)
        {
            var su = new SUnknown("SUnknown.su", null);
            su.cycle = su;
            return su;
        }

        public void checkSUnknown(AnyClass? obj, Current current)
        {
            TestHelper.Assert(obj is SUnknown);
            var su = (SUnknown)obj;
            TestHelper.Assert(su.su.Equals("SUnknown.su"));
        }

        public B oneElementCycle(Current current)
        {
            var b = new B();
            b.sb = "B1.sb";
            b.pb = b;
            return b;
        }

        public B twoElementCycle(Current current)
        {
            var b1 = new B();
            b1.sb = "B1.sb";
            var b2 = new B();
            b2.sb = "B2.sb";
            b2.pb = b1;
            b1.pb = b2;
            return b1;
        }

        public B D1AsB(Current current)
        {
            var d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            var d2 = new D2();
            d2.pb = d1;
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            d2.pd2 = d1;
            d1.pb = d2;
            d1.pd1 = d2;
            return d1;
        }

        public D1 D1AsD1(Current current)
        {
            var d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            var d2 = new D2();
            d2.pb = d1;
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            d2.pd2 = d1;
            d1.pb = d2;
            d1.pd1 = d2;
            return d1;
        }

        public B D2AsB(Current current)
        {
            var d2 = new D2();
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            var d1 = new D1();
            d1.pb = d2;
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            d1.pd1 = d2;
            d2.pb = d1;
            d2.pd2 = d1;
            return d2;
        }

        public (B, B) paramTest1(Current current)
        {
            var d1 = new D1();
            d1.sb = "D1.sb";
            d1.sd1 = "D1.sd1";
            var d2 = new D2();
            d2.pb = d1;
            d2.sb = "D2.sb";
            d2.sd2 = "D2.sd2";
            d2.pd2 = d1;
            d1.pb = d2;
            d1.pd1 = d2;
            return (d1, d2);
        }

        public (B, B) paramTest2(Current current)
        {
            var (p1, p2) = paramTest1(current);
            return (p2, p1);
        }

        public (B, B, B) paramTest3(Current current)
        {
            var d2 = new D2();
            d2.sb = "D2.sb (p1 1)";
            d2.pb = null;
            d2.sd2 = "D2.sd2 (p1 1)";

            var d1 = new D1();
            d1.sb = "D1.sb (p1 2)";
            d1.pb = null;
            d1.sd1 = "D1.sd2 (p1 2)";
            d1.pd1 = null;
            d2.pd2 = d1;

            var d4 = new D2();
            d4.sb = "D2.sb (p2 1)";
            d4.pb = null;
            d4.sd2 = "D2.sd2 (p2 1)";

            var d3 = new D1();
            d3.sb = "D1.sb (p2 2)";
            d3.pb = null;
            d3.sd1 = "D1.sd2 (p2 2)";
            d3.pd1 = null;
            d4.pd2 = d3;

            return (d3, d2, d4);
        }

        public (B, B) paramTest4(Current current)
        {
            var d4 = new D4();
            d4.sb = "D4.sb (1)";
            d4.pb = null;
            d4.p1 = new B();
            d4.p1.sb = "B.sb (1)";
            d4.p2 = new B();
            d4.p2.sb = "B.sb (2)";
            return (d4.p2, d4);
        }

        public (B, B, B) returnTest1(Current current)
        {
            var (p1, p2) = paramTest1(current);
            return (p1, p1, p2);
        }

        public (B, B, B) returnTest2(Current current)
        {
            var (p1, p2) = paramTest1(current);
            return (p2, p2, p1);
        }

        public B? returnTest3(B? p1, B? p2, Current current) => p1;

        public SS3 sequenceTest(SS1? p1, SS2? p2, Current current)
        {
            var ss = new SS3();
            ss.c1 = p1;
            ss.c2 = p2;
            return ss;
        }

        public (IReadOnlyDictionary<int, B?>, IReadOnlyDictionary<int, B?>) dictionaryTest(Dictionary<int, B?> bin,
            Current current)
        {
            var bout = new Dictionary<int, B?>();
            int i;
            for (i = 0; i < 10; ++i)
            {
                B? b = bin[i];
                TestHelper.Assert(b != null);
                var d2 = new D2();
                d2.sb = b.sb;
                d2.pb = b.pb;
                d2.sd2 = "D2";
                d2.pd2 = d2;
                bout[i * 10] = d2;
            }
            var r = new Dictionary<int, B?>();
            for (i = 0; i < 10; ++i)
            {
                string s = "D1." + (i * 20).ToString();
                var d1 = new D1();
                d1.sb = s;
                d1.pb = i == 0 ? null : r[(i - 1) * 20];
                d1.sd1 = s;
                d1.pd1 = d1;
                r[i * 20] = d1;
            }
            return (r, bout);
        }

        public PBase? exchangePBase(PBase? pb, Current current) => pb;

        public Preserved PBSUnknownAsPreserved(Current current) =>
            new PSUnknown(5, "preserved", "unknown", null, new MyClass(15));

        public void checkPBSUnknown(Preserved? p, Current current)
        {

            TestHelper.Assert(p is PSUnknown);
            var pu = (PSUnknown)p;
            TestHelper.Assert(pu.pi == 5);
            TestHelper.Assert(pu.ps.Equals("preserved"));
            TestHelper.Assert(pu.psu.Equals("unknown"));
            TestHelper.Assert(pu.graph == null);
            TestHelper.Assert(pu.cl != null && pu.cl.i == 15);
        }

        public ValueTask<Preserved?>
        PBSUnknownAsPreservedWithGraphAsync(Current current)
        {
            var graph = new PNode();
            graph.next = new PNode();
            graph.next.next = new PNode();
            graph.next.next.next = graph;

            var r = new PSUnknown(5, "preserved", "unknown", graph, null);
            return new ValueTask<Preserved?>(r);
        }

        public void checkPBSUnknownWithGraph(Preserved? p, Current current)
        {
            TestHelper.Assert(p is PSUnknown);
            var pu = (PSUnknown)p;
            TestHelper.Assert(pu.pi == 5);
            TestHelper.Assert(pu.ps.Equals("preserved"));
            TestHelper.Assert(pu.psu.Equals("unknown"));
            TestHelper.Assert(pu.graph != pu.graph!.next);
            TestHelper.Assert(pu.graph.next != pu.graph.next!.next);
            TestHelper.Assert(pu.graph.next.next!.next == pu.graph);
        }

        public ValueTask<Preserved?>
        PBSUnknown2AsPreservedWithGraphAsync(Current current)
        {
            var r = new PSUnknown2(5, "preserved", null);
            r.pb = r;
            return new ValueTask<Preserved?>(r);
        }

        public void checkPBSUnknown2WithGraph(Preserved? p, Current current)
        {
            TestHelper.Assert(p is PSUnknown2);
            var pu = (PSUnknown2)p;
            TestHelper.Assert(pu.pi == 5);
            TestHelper.Assert(pu.ps.Equals("preserved"));
            TestHelper.Assert(pu.pb == pu);
        }

        public PNode? exchangePNode(PNode? pn, Current current) => pn;

        public void throwBaseAsBase(Current current)
        {
            var b = new B("sb", null);
            b.pb = b;
            throw new BaseException("sbe", b);
        }

        public void throwDerivedAsBase(Current current)
        {
            var b = new B("sb1", null);
            b.pb = b;

            var d = new D1("sb2", null, "sd2", null);
            d.pb = d;
            d.pd1 = d;

            throw new DerivedException("sbe", b, "sde1", d);
        }

        public void throwDerivedAsDerived(Current current)
        {
            var b = new B("sb1", null);
            b.pb = b;

            var d = new D1("sb2", null, "sd2", null);
            d.pb = d;
            d.pd1 = d;

            throw new DerivedException("sbe", b, "sde1", d);
        }

        public void throwUnknownDerivedAsBase(Current current)
        {
            var d2 = new D2();
            d2.sb = "sb d2";
            d2.pb = d2;
            d2.sd2 = "sd2 d2";
            d2.pd2 = d2;

            throw new UnknownDerivedException("sbe", d2, "sude", d2);
        }

        public ValueTask
        throwPreservedExceptionAsync(Current current)
        {
            var ue = new PSUnknownException();
            ue.p = new PSUnknown2(5, "preserved", null);
            ue.p.pb = ue.p;
            throw ue;
        }

        public Forward useForward(Current current)
        {
            var f = new Forward();
            f.h = new Hidden();
            f.h.f = f;
            return f;
        }
    }

    public sealed class TestIntf2 : ITestIntf2
    {
        public SBase SBSUnknownDerivedAsSBase(Current current)
            => new SBSUnknownDerived("SBSUnknownDerived.sb", "SBSUnknownDerived.sbsud");

        public void CUnknownAsSBase(SBase? cUnknown, Current current)
        {
            if (cUnknown!.sb != "CUnknown.sb")
            {
                throw new Exception();
            }
        }
    }
}
