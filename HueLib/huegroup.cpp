#include "huegroup.h"

HueGroup::HueGroup(HueBridge* bridge) :
    HueAbstractObject(bridge),
    m_ID(),
    m_action(),
    m_lights(),
    m_sensors(),
    m_state(),
    m_name(),
    m_type(),
    m_groupClass(),
    m_recycle(),
    m_validConstructor(false)
{

}

HueGroup::HueGroup(HueBridge* bridge,
                   int ID,
                   Group::Action action, Group::Lights lights,
                   Group::Sensors sensors, Group::State state,
                   Group::Name name, Group::Type type,
                   Group::GroupClass groupClass, Group::Recycle recycle) :
    HueAbstractObject(bridge),
    m_ID(ID),
    m_action(action),
    m_lights(lights),
    m_sensors(sensors),
    m_state(state),
    m_name(name),
    m_type(type),
    m_groupClass(groupClass),
    m_recycle(recycle),
    m_validConstructor(true)
{

}

HueGroup::HueGroup(const HueGroup& rhs) :
    HueAbstractObject(rhs.getBridge()),
    m_ID(rhs.m_ID),
    m_action(rhs.m_action),
    m_lights(rhs.m_lights),
    m_sensors(rhs.m_sensors),
    m_state(rhs.m_state),
    m_name(rhs.m_name),
    m_type(rhs.m_type),
    m_groupClass(rhs.m_groupClass),
    m_recycle(rhs.m_recycle),
    m_validConstructor(rhs.m_validConstructor)
{

}

HueGroup HueGroup::operator=(const HueGroup &rhs)
{
    if (&rhs == this)
        return *this;

    this->setBridge(rhs.getBridge());
    this->setParent(rhs.parent());
    m_ID = rhs.m_ID;
    m_action = rhs.m_action;
    m_lights = rhs.m_lights;
    m_sensors = rhs.m_sensors;
    m_state = rhs.m_state;
    m_name = rhs.m_name;
    m_type = rhs.m_type;
    m_groupClass = rhs.m_groupClass;
    m_recycle = rhs.m_recycle;
    m_validConstructor = rhs.m_validConstructor;

    return *this;
}


QList<HueGroup*> HueGroup::discoverGroups(HueBridge *bridge)
{
    QList<HueGroup*> groups;

    HueRequest request("groups", QJsonObject(), HueRequest::get);
    HueReply reply = bridge->sendRequest(request);

    if (!reply.isValid() || reply.timedOut()) {
        qDebug() << "INVALID HUE REPLY";
        qDebug() << "=============================";
        qDebug() << "Is valid: " << reply.isValid();
        qDebug() << "Timed out: " << reply.timedOut();
        qDebug() << "HTTP status: " << reply.getHttpStatus();
        qDebug() << "Error type: " << reply.getErrorType();
        qDebug() << "Error address: " << reply.getErrorAddress();
        qDebug() << "Error description: " << reply.getErrorDescription();
        qDebug() << "=============================";

        return groups;
    }

    QJsonObject json = reply.getJson();
    auto parsedJsonWithIDs = HueAbstractObject::parseJson(json);

    if (parsedJsonWithIDs.isEmpty())
        return groups;

    for (auto iter = parsedJsonWithIDs.begin(); iter != parsedJsonWithIDs.end(); ++iter) {
        int ID = iter.key();
        QJsonObject json = iter.value();

        HueGroup* group = new HueGroup(bridge);

        if (HueGroup::constructHueGroup(ID, json, group))
            groups.append(group);
        else
            delete group;
    }

    return groups;
}

bool HueGroup::hasValidConstructor() const
{
    return m_validConstructor;
}

int HueGroup::ID() const
{
    return m_ID;
}

Group::Action HueGroup::action() const
{
    return m_action;
}

Group::Lights HueGroup::lights() const
{
    return m_lights;
}

Group::Sensors HueGroup::sensors() const
{
    return m_sensors;
}

Group::State HueGroup::state() const
{
    return m_state;
}

Group::Name HueGroup::name() const
{
    return m_name;
}

Group::Type HueGroup::type() const
{
    return m_type;
}

Group::GroupClass HueGroup::groupClass() const
{
    return m_groupClass;
}

Group::Recycle HueGroup::recycle() const
{
    return m_recycle;
}

bool HueGroup::constructHueGroup(int ID, QJsonObject json, HueGroup* group)
{
    bool jsonIsValid =
            json.contains("name")       &
            json.contains("lights")     &
            json.contains("sensors")    &
            json.contains("type")       &
            json.contains("state")      &
            json.contains("recycle")    &
            json.contains("class")      &
            json.contains("action")     ;

    if(!jsonIsValid)
        return false;

    Group::Action action(json["action"]);
    Group::Lights lights(json["lights"]);
    Group::Sensors sensors(json["sensors"]);
    Group::State state(json["state"]);
    Group::Name name(json["name"]);
    Group::Type type(json["type"]);
    Group::GroupClass groupClass(json["class"]);
    Group::Recycle recycle(json["recycle"]);

    HueGroup newGroup(group->getBridge(), ID, action, lights, sensors,
                      state, name, type, groupClass, recycle);

    *group = newGroup;

    return true;
}

bool HueGroup::synchronize()
{
    HueRequest syncRequest = makeGetRequest();
    HueReply syncReply;

    bool replyValid = sendRequest(syncRequest, syncReply);

    if (replyValid) {
        QJsonObject json = syncReply.getJson();
        HueGroup synchronizedGroup(getBridge());

        if (constructHueGroup(m_ID, json, &synchronizedGroup)) {
            if (synchronizedGroup.hasValidConstructor()) {
                *this = synchronizedGroup;
                return true;
            }
        }
    }

    return false;
}

HueRequest HueGroup::makePutRequest(QJsonObject json)
{
    QString urlPath = "groups/" + QString::number(m_ID) + "/action";
    HueRequest::Method method = HueRequest::put;

    return HueRequest(urlPath, json, method);
}

HueRequest HueGroup::makeGetRequest()
{
    QString urlPath = "groups/" + QString::number(m_ID);
    HueRequest::Method method = HueRequest::get;

    return HueRequest(urlPath, QJsonObject(), method);
}